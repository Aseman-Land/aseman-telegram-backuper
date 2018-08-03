#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointer>
#include <QLabel>
#include <QMimeDatabase>
#include <QDialog>

#include <functional>
#include <telegram.h>

namespace Ui {
class MainWindow;
}

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
    QVariantList mMessagesList;
    MessagesStickerSet mSticketSet;
};

#endif // MAINWINDOW_H
