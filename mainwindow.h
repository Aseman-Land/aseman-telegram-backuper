#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointer>
#include <QLabel>
#include <QMimeDatabase>
#include <QDialog>
#include <QDateTime>
#include <QCryptographicHash>

#include <functional>
#include <telegram.h>

namespace Ui {
class MainWindow;
}

class TGB_Peer {
public:
    qint32 count = 0;
    qint32 charCount = 0;
    qint32 delay = 0;
    qint32 smallDelay = 0;
    qint32 longDelay = 0;
    qint32 delayPerSession = 0;
    qint32 smallDelayPerSession = 0;
    qint32 longDelayPerSession = 0;
    qint32 stickers = 0;
    qint32 emojiMessagesCount = 0;
    qint32 emojisCount = 0;
    qint32 messageSessionsCount = 1;
    qint32 voiceDuration = 0;
    qint32 voiceCount = 0;
    qint32 roundDuration = 0;
    qint32 roundCount = 0;
    qint32 imageCount = 0;
    QMap<QString, qint32> stickerPerChar;

    qint32 maxDelay = 0;
    qint32 minDelay = 0;
    qint32 maxDelayPerSession = 0;
    qint32 minDelayPerSession = 0;

    qreal percentContaintsEmoji = 0;
    qreal percentRound = 0;
    qreal percentVoice= 0;

    qreal avgCharCount = 0;
    qreal avgDelay = 0;
    qreal avgSmallDelay = 0;
    qreal avgLongDelay = 0;
    qreal avgDelayPerSession = 0;
    qreal avgSmallDelayPerSession = 0;
    qreal avgLongDelayPerSession = 0;
    qreal avgContiniusMessages = 0;
    qreal avgEmojiPerMessage = 0;
    qreal avgVoiceDuration = 0;
    qreal avgRoundDuration = 0;

    QMap<QString, qint32> usedEmojis;

    QVariantMap toMap() const {
        QVariantMap res;
        res["count"] = count;
        res["charCount"] = charCount;
        res["delay"] = delay;
        res["smallDelay"] = smallDelay;
        res["longDelay"] = longDelay;
        res["delayPerSession"] = delayPerSession;
        res["smallDelayPerSession"] = smallDelayPerSession;
        res["longDelayPerSession"] = longDelayPerSession;
        res["stickers"] = stickers;
        res["emojisCount"] = emojisCount;
        res["emojiMessagesCount"] = emojiMessagesCount;
        res["messageSessionsCount"] = messageSessionsCount;
        res["voiceDuration"] = voiceDuration;
        res["voiceCount"] = voiceCount;
        res["roundDuration"] = roundDuration;
        res["roundCount"] = roundCount;
        res["imageCount"] = imageCount;

        res["maxDelay"] = maxDelay;
        res["minDelay"] = minDelay;
        res["maxDelayPerSession"] = maxDelayPerSession;
        res["minDelayPerSession"] = minDelayPerSession;

        res["percentContaintsEmoji"] = percentContaintsEmoji;
        res["percentRound"] = percentRound;
        res["percentVoice"] = percentVoice;

        res["avgPerMessageCharCount"] = avgCharCount;
        res["avgDelay"] = avgDelay;
        res["avgSmallDelay"] = avgSmallDelay;
        res["avgLongDelay"] = avgLongDelay;
        res["avgDelayPerSession"] = avgDelayPerSession;
        res["avgSmallDelayPerSession"] = avgSmallDelayPerSession;
        res["avgLongDelayPerSession"] = avgLongDelayPerSession;
        res["avgContiniusMessages"] = avgContiniusMessages;
        res["avgEmojiPerMessage"] = avgEmojiPerMessage;
        res["avgVoiceDuration"] = avgVoiceDuration;
        res["avgRoundDuration"] = avgRoundDuration;

        QVariantMap emojis;
        QMapIterator<QString, qint32> i(usedEmojis);
        while (i.hasNext())
        {
            i.next();
            emojis[i.key()] = i.value();
        }
        res["usedEmojis"] = emojis;

        QVariantMap stickers;
        QMapIterator<QString, qint32> j(stickerPerChar);
        while (j.hasNext())
        {
            j.next();
            stickers[j.key()] = j.value();
        }
        res["stickerPerChar"] = stickers;

        return res;
    }
};

class TGB_Month {
public:
    TGB_Peer in;
    TGB_Peer out;
    TGB_Peer sum;

    QVariantMap toMap() const {
        QVariantMap res;
        res["in"] = in.toMap();
        res["out"] = out.toMap();
        res["sum"] = sum.toMap();
        return res;
    }
};

class TGB_Chat {
public:
    QString name;
    QString fromHash;
    QString toHash;
    QString label = "none";
    QMap<QDate, TGB_Month> months;

    QVariantMap toMap() const {
        QVariantMap res;
        res["name"] = name;
        res["label"] = label;
        res["fromHash"] = fromHash;
        res["toHash"] = toHash;

        QVariantMap map;
        QMapIterator<QDate, TGB_Month> i(months);
        while (i.hasNext())
        {
            i.next();
            map[i.key().toString("yyyy-MM")] = i.value().toMap();
        }
        res["months"] = map;

        return res;
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static QString homePath();

private slots:
    void on_nextBtn_clicked();
    void on_downloadBtn_clicked();
    void on_actionBack_triggered();
    void on_cancelBtn_clicked();
    void on_stackedWidget_currentChanged(int arg1);
    void on_actionProxy_triggered();
    void on_resetBtn_clicked();
    void on_actionAbout_triggered();
    void on_dialogBtn_clicked();

private:
    void initTelegram(const QString &phoneNumber);
    void initProxy();
    void doSendCode(const QString &code);
    void doCheckPassword(const QString &password);
    void getChannelDetails(const QString &name);
    void downloadMessages(const InputPeer &peer, qint32 offset_id = 0, qint32 offset_date = 0, qint32 offset = 0);
    void downloadMedias(QList<Message> msg, qint32 offset, qint32 count, std::function<void ()> callback);
    void downloadDocuments(QList<Document> docs, qint32 offset = 0);
    void finish();

private:
    QString fileExists(const QString &path);
    void waitLabelHide();
    void waitLabelShow();
    void initEmojis();

    QVariantList convertToList() const;
    void insertMonth(TGB_Peer &tgb, const Message &msg, const Message &prev) const;

private:
    Ui::MainWindow *ui;

    Telegram *mTg;
    QMimeDatabase mMimeDb;

    User mUser;
    Chat mChat;

    QString mCurrentSalt;
    qint64 mTotalDownloaded;
    qint32 mLimit;
    QHash<qint64, int> mFilesTimeoutCount;
    QString mDestination;
    MessagesStickerSet mSticketSet;

    QVariantList mMessagesList;
    QMap<qint32, QMap<QDate, QMap<qint32, Message> > > mMessages;
    QHash<qint32, User> mUsers;
    QHash<qint32, Chat> mChats;

    QHash<QString, QString> mEmojis;
};

#endif // MAINWINDOW_H
