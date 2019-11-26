#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_proxy.h"
#include "ui_about.h"
#include "ui_dialog.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QMessageBox>
#include <QStringListModel>
#include <QTimer>
#include <QMimeType>
#include <QDateTime>
#include <QFileDialog>
#include <QNetworkProxy>
#include <QDesktopServices>
#include <QSettings>
#include <QtMath>

static QString *aseman_app_home_path = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mTg(Q_NULLPTR)
{
    ui->setupUi(this);

    QStringListModel *phonesModel = new QStringListModel(ui->phoneLine);
    phonesModel->setStringList( QDir(homePath()).entryList(QDir::Dirs | QDir::NoDotAndDotDot) );

    ui->phoneLine->setModel(phonesModel);
    ui->stackedWidget->setCurrentIndex(0);

    waitLabelHide();
    initProxy();
    initEmojis();
}

void MainWindow::initProxy()
{
    QSettings settings(homePath() + "/configs.ini", QSettings::IniFormat);

    qint32 type = settings.value("Proxy/type", 0).toInt();
    QNetworkProxy newProxy;
    switch(type)
    {
    case 0: // No Proxy
        newProxy.setType(QNetworkProxy::NoProxy);
        QNetworkProxy::setApplicationProxy(newProxy);
        break;
    case 1: // Http
        newProxy.setType(QNetworkProxy::HttpProxy);
        newProxy.setHostName( settings.value("Proxy/host").toString() );
        newProxy.setPort( settings.value("Proxy/port").toInt() );
        newProxy.setUser( settings.value("Proxy/user").toString() );
        newProxy.setPassword( settings.value("Proxy/pass").toString() );
        QNetworkProxy::setApplicationProxy(newProxy);
        break;
    case 2: // Socks5
        newProxy.setType(QNetworkProxy::Socks5Proxy);
        newProxy.setHostName( settings.value("Proxy/host").toString() );
        newProxy.setPort( settings.value("Proxy/port").toInt() );
        newProxy.setUser( settings.value("Proxy/user").toString() );
        newProxy.setPassword( settings.value("Proxy/pass").toString() );
        QNetworkProxy::setApplicationProxy(newProxy);
        break;
    }
}

void MainWindow::on_stackedWidget_currentChanged(int)
{
    switch(ui->stackedWidget->currentIndex())
    {
    case 0: // PhoneNumber
        ui->cancelBtn->hide();
        ui->downloadBtn->hide();
        ui->nextBtn->show();
        ui->resetBtn->hide();
        ui->toolBar->show();
        ui->actionBack->setEnabled(false);
        ui->actionProxy->setEnabled(true);
        break;

    case 1: // Code
    case 2: // Password
    case 3: // Channel
        ui->cancelBtn->hide();
        ui->downloadBtn->hide();
        ui->nextBtn->show();
        ui->resetBtn->hide();
        ui->toolBar->show();
        ui->actionBack->setEnabled(true);
        ui->actionProxy->setEnabled(false);
        break;

    case 4: // Details
        ui->cancelBtn->hide();
        ui->downloadBtn->show();
        ui->nextBtn->hide();
        ui->resetBtn->hide();
        ui->toolBar->show();
        ui->resetBtn->hide();
        ui->actionBack->setEnabled(true);
        ui->actionProxy->setEnabled(false);
        break;

    case 5: // Download
        ui->cancelBtn->show();
        ui->downloadBtn->hide();
        ui->nextBtn->hide();
        ui->resetBtn->hide();
        ui->toolBar->hide();
        break;

    case 6: // Finished
        ui->cancelBtn->hide();
        ui->downloadBtn->hide();
        ui->nextBtn->hide();
        ui->resetBtn->show();
        ui->toolBar->hide();
        break;
    }
}

void MainWindow::on_actionBack_triggered()
{
    if(ui->stackedWidget->currentIndex() == 0)
        return;

    if(ui->stackedWidget->currentIndex() == 3)
        ui->stackedWidget->setCurrentIndex(0);
    else
        ui->stackedWidget->setCurrentIndex( ui->stackedWidget->currentIndex()-1 );

    if(ui->stackedWidget->currentIndex() == 0)
    {
        mTg->deleteLater();
        mTg = Q_NULLPTR;
    }
}

void MainWindow::on_cancelBtn_clicked()
{
    ui->stackedWidget->setCurrentIndex(4);
    initTelegram(ui->phoneLine->currentText());
}

void MainWindow::on_resetBtn_clicked()
{
    on_cancelBtn_clicked();
}

void MainWindow::on_actionAbout_triggered()
{
    QDialog *about = new QDialog(this);
    Ui::About *aboutUi = new Ui::About;
    aboutUi->setupUi(about);

    connect(aboutUi->githubBtn, &QPushButton::clicked, this, [this](){
        QDesktopServices::openUrl( QUrl("https://github.com/Aseman-Land/aseman-telegram-backuper") );
    });

    about->setWindowModality(Qt::ApplicationModal);
    about->exec();

    connect(about, &QDialog::destroyed, this, [this, aboutUi](){
        delete aboutUi;
    });
    about->deleteLater();
}

void MainWindow::on_dialogBtn_clicked()
{
    waitLabelShow();
    mTg->messagesGetDialogs(false, false, false, InputPeer::null, 100, [this](TG_MESSAGES_GET_DIALOGS_CALLBACK){
        Q_UNUSED(msgId)
        waitLabelHide();
        if(!error.null) {
            QMessageBox::critical(this, "Can't get the list", error.errorText);
            return;
        }

        QDialog *dialog = new QDialog(this);
        Ui::Dialog *dialogUi = new Ui::Dialog;
        dialogUi->setupUi(dialog);

        QHash<qint32, User> users;
        for(const User &user: result.users())
            users[user.id()] = user;
        QHash<qint32, Chat> chats;
        for(const Chat &chat: result.chats())
            chats[chat.id()] = chat;
        for(const Dialog &dlg: result.dialogs())
        {
            QListWidgetItem *item = new QListWidgetItem();
            switch(static_cast<qint32>(dlg.peer().classType()))
            {
            case Peer::typePeerChannel:
            {
                Chat chat = chats.value(dlg.peer().channelId());
                item->setData(Qt::UserRole+1, QVariant::fromValue<Chat>(chat));
                item->setText(chat.title());
                item->setIcon(QIcon(":/icons/globe.png"));
            }
                break;
            case Peer::typePeerChat:
            {
                Chat chat = chats.value(dlg.peer().chatId());
                item->setData(Qt::UserRole+1, QVariant::fromValue<Chat>(chat));
                item->setText(chat.title());
                item->setIcon(QIcon(":/icons/user-group-new.png"));
            }
                break;
            case Peer::typePeerUser:
            {
                User user = users.value(dlg.peer().userId());
                item->setData(Qt::UserRole+2, QVariant::fromValue<User>(user));
                item->setText( (user.firstName() + " " + user.lastName()).trimmed() );
                item->setIcon(QIcon(":/icons/im-user.png"));
            }
                break;
            }

            dialogUi->listWidget->addItem(item);
        }

        connect(dialogUi->listWidget, &QListWidget::doubleClicked, dialog, &QDialog::accept);
        connect(dialog, &QDialog::accepted, this, [this, dialogUi](){
            if(dialogUi->listWidget->selectedItems().isEmpty())
                return;

            QListWidgetItem *item = dialogUi->listWidget->currentItem();

            mChat = item->data(Qt::UserRole+1).value<Chat>();
            mUser = item->data(Qt::UserRole+2).value<User>();

            if(mChat.title().count())
            {
                ui->channelIdLine->setText( mChat.title() + ": " + QString::number(mChat.id()) );
                ui->diaryCheck->setDisabled(true);
                ui->diaryCheck->setChecked(false);
            }
            else
            {
                ui->channelIdLine->setText( (mUser.firstName() + " " + mUser.lastName()).trimmed() + ": " + QString::number(mUser.id()) );
                ui->diaryCheck->setEnabled(true);
                ui->diaryCheck->setChecked(true);
            }

            ui->stackedWidget->setCurrentIndex(4);
        });

        dialog->setWindowModality(Qt::ApplicationModal);
        dialog->exec();

        connect(dialog, &QDialog::destroyed, this, [this, dialogUi](){
            delete dialogUi;
        });
        dialog->deleteLater();
    });
}

void MainWindow::on_actionProxy_triggered()
{
    QDialog *proxy = new QDialog(this);
    Ui::Proxy *proxyUi = new Ui::Proxy;
    proxyUi->setupUi(proxy);

    QNetworkProxy current = QNetworkProxy::applicationProxy();
    switch( static_cast<qint32>(current.type()) )
    {
    case QNetworkProxy::NoProxy:
        proxyUi->type->setCurrentIndex(0);
        break;
    case QNetworkProxy::HttpProxy:
        proxyUi->type->setCurrentIndex(1);
        break;
    case QNetworkProxy::Socks5Proxy:
        proxyUi->type->setCurrentIndex(2);
        break;
    }
    proxyUi->host->setText(current.hostName());
    proxyUi->port->setValue(current.port());
    proxyUi->user->setText(current.user());
    proxyUi->pass->setText(current.password());

    proxy->setWindowModality(Qt::ApplicationModal);

    connect(proxy, &QDialog::accepted, this, [this, proxy, proxyUi](){
        QNetworkProxy newProxy;
        switch(proxyUi->type->currentIndex())
        {
        case 0: // No Proxy
            newProxy.setType(QNetworkProxy::NoProxy);
            QNetworkProxy::setApplicationProxy(newProxy);
            break;
        case 1: // Http
            newProxy.setType(QNetworkProxy::HttpProxy);
            newProxy.setHostName(proxyUi->host->text());
            newProxy.setPort(proxyUi->port->value());
            newProxy.setUser(proxyUi->user->text());
            newProxy.setPassword(proxyUi->pass->text());
            QNetworkProxy::setApplicationProxy(newProxy);
            break;
        case 2: // Socks5
            newProxy.setType(QNetworkProxy::Socks5Proxy);
            newProxy.setHostName(proxyUi->host->text());
            newProxy.setPort(proxyUi->port->value());
            newProxy.setUser(proxyUi->user->text());
            newProxy.setPassword(proxyUi->pass->text());
            QNetworkProxy::setApplicationProxy(newProxy);
            break;
        }

        QSettings settings(homePath() + "/configs.ini", QSettings::IniFormat);
        settings.setValue("Proxy/type", proxyUi->type->currentIndex());
        settings.setValue("Proxy/host", proxyUi->host->text());
        settings.setValue("Proxy/port", proxyUi->port->value());
        settings.setValue("Proxy/user", proxyUi->user->text());
        settings.setValue("Proxy/pass", proxyUi->pass->text());
    }, Qt::QueuedConnection);

    proxy->exec();

    connect(proxy, &QDialog::destroyed, this, [this, proxyUi](){
        delete proxyUi;
    });

    proxy->deleteLater();
}

void MainWindow::on_nextBtn_clicked()
{
    switch(ui->stackedWidget->currentIndex())
    {
    case 0: // PhoneNumber
        initTelegram(ui->phoneLine->currentText());
        break;

    case 1: // Code
        doSendCode(ui->codeLine->text());
        break;

    case 2: // Password
        doCheckPassword(ui->passLine->text());
        break;

    case 3: // Channel
        getChannelDetails(ui->channelName->text());
        break;

    case 4: // Details
        on_downloadBtn_clicked();
        break;

    case 5: // Download
        break;

    case 6: // Finished
        break;
    }
}

void MainWindow::on_downloadBtn_clicked()
{
    ui->stackedWidget->setCurrentIndex(5);
    ui->progressBar->setValue(0);
    ui->progressLabel->setText(tr("Please wait..."));

    InputPeer peer;
    if(mUser.classType() == User::typeUser)
    {
        peer.setClassType(InputPeer::typeInputPeerUser);
        peer.setAccessHash(mUser.accessHash());
        peer.setUserId(mUser.id());
    }
    else
    if(mChat.classType() == Chat::typeChat)
    {
        peer.setClassType(InputPeer::typeInputPeerChat);
        peer.setAccessHash(mChat.accessHash());
        peer.setChatId(mChat.id());
    }
    else
    if(mChat.classType() == Chat::typeChannel)
    {
        peer.setClassType(InputPeer::typeInputPeerChannel);
        peer.setAccessHash(mChat.accessHash());
        peer.setChannelId(mChat.id());
    }

    mCurrentSalt.clear();
    mTotalDownloaded = 0;
    mLimit = ui->limitSpin->value()? ui->limitSpin->value() : -1;
    mFilesTimeoutCount.clear();
    mMessages.clear();
    mDestination.clear();
    ui->sizeLabel->setText("");

    if(mSticketSet.documents().count())
        downloadDocuments(mSticketSet.documents());
    else
        downloadMessages(peer);
}

void MainWindow::initTelegram(const QString &phoneNumber)
{
    if(mTg)
        mTg->deleteLater();

    mTg = new Telegram("149.154.167.50", 443, 2,  13682, "de37bcf00f4688de900510f4f87384bb", phoneNumber,
                      homePath() + "/",
                      ":/tg-server.pub");

    connect(mTg, &Telegram::authLoggedIn, this, [this](){
        ui->stackedWidget->setCurrentIndex(3);
        waitLabelHide();
    });
    connect(mTg, &Telegram::authNeeded, this, [this](){
        ui->stackedWidget->setCurrentIndex(1);
        waitLabelShow();

        mTg->authSendCode([this](TG_AUTH_SEND_CODE_CALLBACK){
            Q_UNUSED(msgId)
            waitLabelHide();
            if(!error.null) {
                QMessageBox::critical(this, "Send code error", error.errorText);
                ui->stackedWidget->setCurrentIndex(0);
                return;
            }
        });
    });

    waitLabelShow();
    mTg->init();
}

void MainWindow::doSendCode(const QString &code)
{
    if(!mTg)
    {
        ui->stackedWidget->setCurrentIndex(0);
        return;
    }

    waitLabelShow();
    mTg->authSignIn(code, [this](TG_AUTH_SIGN_IN_CALLBACK){
        Q_UNUSED(msgId)
        if(error.errorText == "SESSION_PASSWORD_NEEDED") {
            mTg->accountGetPassword([this](TG_ACCOUNT_GET_PASSWORD_CALLBACK){
                Q_UNUSED(msgId)
                waitLabelHide();
                if(!error.null) {
                    QMessageBox::critical(this, "Get password error", error.errorText);
                    ui->stackedWidget->setCurrentIndex(0);
                    return;
                }

                //As a workaround for the binary corruption of the AccountPassword we store it here as a string, thereby guaranteeing deep copy
                mCurrentSalt = result.currentSalt().toHex();
                ui->stackedWidget->setCurrentIndex(2);
            });
        }
        else
        if(!error.null) {
            waitLabelHide();
            QMessageBox::critical(this, "Invalid code", error.errorText);
            ui->stackedWidget->setCurrentIndex(1);
            return;
        }
    });
}

void MainWindow::doCheckPassword(const QString &password)
{
    waitLabelShow();
    //Reconstructing a byte array with the current salt. A better solution could be made of course
    const QByteArray salt = QByteArray::fromHex(mCurrentSalt);
    QByteArray passData = salt + password.toUtf8() + salt;

    //Moved hash calculation here. Either the whole hash is done here or in the lower lib. Splitting makes things hard to analyze & debug
    mTg->authCheckPassword(QCryptographicHash::hash(passData, QCryptographicHash::Sha256), [this](TG_AUTH_CHECK_PASSWORD_CALLBACK){
        Q_UNUSED(msgId)
        Q_UNUSED(result)
        waitLabelHide();
        if(!error.null) {
            QMessageBox::critical(this, "Invalid password", error.errorText);
            ui->stackedWidget->setCurrentIndex(2);
            return;
        }

        ui->stackedWidget->setCurrentIndex(3);
    });
}

void MainWindow::getChannelDetails(const QString &name)
{
    if(!mTg)
    {
        ui->stackedWidget->setCurrentIndex(0);
        return;
    }

    mUser = User::typeUserEmpty;
    mChat = Chat::typeChatEmpty;
    mSticketSet = MessagesStickerSet();

    waitLabelShow();
    mTg->contactsResolveUsername(name, [this, name](TG_CONTACTS_RESOLVE_USERNAME_CALLBACK){
        waitLabelHide();

        if(!error.null) {
            // Maybe Its sticker?
            QString errorText = error.errorText;

            InputStickerSet input;
            input.setClassType(InputStickerSet::typeInputStickerSetShortName);
            input.setShortName(name);
            mTg->messagesGetStickerSet(input, [this, errorText](TG_MESSAGES_GET_STICKER_SET_CALLBACK){
                if(!error.null) {
                    // Maybe Its sticker?
                    QMessageBox::critical(this, "Invalid Channel", errorText);
                    ui->stackedWidget->setCurrentIndex(3);
                    return;
                }

                mSticketSet = result;
                ui->channelIdLine->setText("Its sticker: " + mSticketSet.set().title());
                ui->optionsWidget->hide();
                ui->stackedWidget->setCurrentIndex(4);
            });
            return;
        }

        if(result.chats().count())
        {
            mChat = result.chats().first();
            ui->channelIdLine->setText( mChat.title() + ": " + QString::number(mChat.id()) );
            ui->optionsWidget->show();
            ui->diaryCheck->setDisabled(true);
            ui->diaryCheck->setChecked(false);
        }
        else
        if(result.users().count())
        {
            mUser = result.users().first();
            ui->channelIdLine->setText( (mUser.firstName() + " " + mUser.lastName()).trimmed() + ": " + QString::number(mUser.id()) );
            ui->optionsWidget->show();
            ui->diaryCheck->setEnabled(true);
            ui->diaryCheck->setChecked(true);
        }

        ui->stackedWidget->setCurrentIndex(4);
    });

}

void MainWindow::downloadMessages(const InputPeer &peer, qint32 offset_id, qint32 offset_date, qint32 offset)
{
    if(mLimit == 0)
    {
        finish();
        return;
    }

    if(mDestination.isEmpty())
    {
        mDestination = QFileDialog::getExistingDirectory(this, tr("Please enter path to save files"));
        if(mDestination.isEmpty())
        {
            finish();
            return;
        }
    }

    class BackupState {
    public:
        InputPeer peer;
        qint32 offset_id;
        qint32 offset_date;
        qint32 offset;
    };

    BackupState backup;
    backup.peer = peer;
    backup.offset_id = offset_id;
    backup.offset_date = offset_date;
    backup.offset = offset;

    mTg->messagesGetHistory(peer, offset_id, offset_date, 0, 100, 0, 0, [this, offset, peer, backup](TG_MESSAGES_GET_HISTORY_CALLBACK){
        if(!error.null) {
            qDebug() << error.errorText;
            QTimer::singleShot(5000, [this, backup](){
                downloadMessages(backup.peer, backup.offset_id, backup.offset_date, backup.offset);
            });
            return;
        }

        if(result.messages().isEmpty() || QDateTime::fromTime_t(result.messages().last().date()).date() < ui->minimumDate->date())
        {
            finish();
            return;
        }

        QHash<qint32, User> users;
        for(const User &user: result.users())
            users[user.id()] = user;

        QList<Message> messages;
        for(const Message &msg: result.messages())
        {
            messages << msg;

            QVariantMap map;
            map["text"] = msg.media().caption().count()? msg.media().caption() : msg.message();
            map["isMedia"] = (msg.media().classType() != MessageMedia::typeMessageMediaEmpty);
            map["id"] = msg.id();
            map["date"] = QDateTime::fromTime_t(msg.date());
            map["out"] = msg.out();
            if(msg.fromId())
            {
                map["fromId"] = msg.fromId();
                map["user"] = (users.value(msg.fromId()).firstName() + " " + users.value(msg.fromId()).lastName()).trimmed();
            }

            QDateTime msgDate = QDateTime::fromTime_t(msg.date());
            QDate indexDate = QDate(msgDate.date().year(), qFloor((msgDate.date().month()-1)/2.0)*2 + 1, 1);

            for (const User &u: result.users())
                mUsers[u.id()] = u;
            for (const Chat &c: result.chats())
                mChats[c.id()] = c;

            mMessages[peer.userId()? peer.userId() : peer.chatId()][indexDate][msg.id()] = msg;
            mMessagesList << map;

            if(mLimit != -1)
            {
                mLimit--;
                if(mLimit == 0)
                    break;
            }
        }

        const qint32 newOffset = offset + messages.length();
        const qint32 count = ui->limitSpin->value()? ui->limitSpin->value() : result.count();

        downloadMedias(messages, newOffset, count, [this, offset, peer, messages, newOffset, count](){
            QTimer::singleShot(500, this, [this, offset, peer, messages, newOffset](){
                Message msg = messages.last();
                downloadMessages(peer, msg.id(), msg.date(), newOffset);
            });

            if(count) ui->progressBar->setValue( 1000*newOffset / count );
        });
    });
}

void MainWindow::downloadMedias(QList<Message> msgs, qint32 offset, qint32 count, std::function<void ()> callback)
{
    if(!ui->mediaCheck->isChecked() || msgs.isEmpty())
    {
        callback();
        return;
    }

    const Message &msg = msgs.takeFirst();
    const MessageMedia &media = msg.media();

    qint32 size = 0;
    qint32 dcId = 0;
    QString fileName = QString::number(msg.id());
    InputFileLocation input(InputFileLocation::typeInputFileLocation);
    switch(static_cast<int>(media.classType()))
    {
    case MessageMedia::typeMessageMediaDocument:
        input.setClassType(InputFileLocation::typeInputDocumentFileLocation);
        input.setId(media.document().id());
        input.setAccessHash(media.document().accessHash());
        size = media.document().size();
        dcId = media.document().dcId();
        break;
    case MessageMedia::typeMessageMediaPhoto:
    {
        const PhotoSize &photo = media.photo().sizes().last();
        input.setClassType(InputFileLocation::typeInputFileLocation);
        input.setLocalId(photo.location().localId());
        input.setSecret(photo.location().secret());
        input.setVolumeId(photo.location().volumeId());
        size = photo.size();
        dcId = photo.location().dcId();
    }
        break;

    default:
        downloadMedias(msgs, offset, count, callback);
        return;
        break;
    }

    ui->progressLabel->setText( QString("There are %1 media to download")
                                .arg(msgs.count()) );

    QString relativeDir = QDateTime::fromTime_t(msg.date()).toString("yyyy-MM");
    QString relativeFile = relativeDir + "/" + fileName;
    const QString &filePath = mDestination + "/" + relativeFile;

    QDir().mkpath(mDestination + "/" + relativeDir);
    const QString &existingFile = fileExists(filePath);
    if(!existingFile.isEmpty())
    {
        ui->progressLabel->setText( QString("\"%1\" already downloaded.").arg(existingFile));
        downloadMedias(msgs, offset, count, callback);
        return;
    }

    QFile *file = new QFile(filePath, this);
    if(!file->open(QFile::WriteOnly))
    {
        delete file;
        downloadMedias(msgs, offset, count, callback);
        return;
    }

    if(!mFilesTimeoutCount.contains(msg.id()))
        mFilesTimeoutCount[msg.id()] = 0;

    const int currentTry = mFilesTimeoutCount.value(msg.id());

    QTimer *timer = new QTimer(this);
    timer->setInterval(60000);
    timer->setSingleShot(true);

    mTg->uploadGetFile(input, size, dcId, [=](TG_UPLOAD_GET_FILE_CUSTOM_CALLBACK){
        Q_UNUSED(msgId)
        if(!mFilesTimeoutCount.contains(msg.id()) ||
            mFilesTimeoutCount.value(msg.id()) != currentTry)
            return;
        if(!error.null) {
            mFilesTimeoutCount.remove(msg.id());
            QTimer::singleShot(50, this, [=](){
                downloadMedias(msgs, offset, count, callback);
            });
            file->close();
            file->remove();
            file->deleteLater();
            timer->deleteLater();
            return;
        }

        mTotalDownloaded += result.bytes().size();
        ui->sizeLabel->setText( QString("%1KB downloaded").arg(mTotalDownloaded/1000) );

        switch ( static_cast<qint64>(result.classType()) ) {
        case UploadGetFile::typeUploadGetFileCanceled:
        case UploadGetFile::typeUploadGetFileFinished: {
            mFilesTimeoutCount.remove(msg.id());
            file->write(result.bytes());
            file->close();
            file->deleteLater();
            timer->deleteLater();

            QMimeType mime = mMimeDb.mimeTypeForFile(filePath);
            if(!mime.suffixes().isEmpty())
            {
                QString newName = filePath + "." + mime.suffixes().first();
                QFile::rename(filePath, newName);
                ui->progressLabel->setText( QString("\"%1\" downloaded successfully.")
                                            .arg(fileName + "." + mime.suffixes().first()));
            }
            else
            {
                ui->progressLabel->setText( QString("\"%1\" downloaded successfully.")
                                            .arg(fileName));
            }

            QTimer::singleShot(50, this, [=](){
                downloadMedias(msgs, offset, count, callback);
            });
        }
            break;

        case UploadGetFile::typeUploadGetFileProgress:
            file->write(result.bytes());
            timer->stop();
            timer->start();
            break;
        }
    });

    connect(timer, &QTimer::timeout, this, [=](){
        if(!mFilesTimeoutCount.contains(msg.id()))
            return;
        file->close();
        file->remove();
        delete file;

        const int retriesLimit = 3;

        mFilesTimeoutCount[msg.id()]++;
        if(mFilesTimeoutCount.value(msg.id()) >= retriesLimit)
        {
            ui->progressLabel->setText( QString("Error: Failed to download \"%1\"").arg(fileName) );
            mFilesTimeoutCount.remove(msg.id());
            downloadMedias(msgs, offset, count, callback);
        }
        else
        {
            ui->progressLabel->setText( "Timed out. Try again..." );
            downloadMedias(msgs, offset, count, callback);
        }
    });
    timer->start();

    if(count)
        ui->progressBar->setValue( 1000*(offset - msgs.length()) / count);
}

void MainWindow::downloadDocuments(QList<Document> docs, qint32 offset)
{
    if(docs.isEmpty())
    {
        finish();
        return;
    }

    if(mDestination.isEmpty())
    {
        mDestination = QFileDialog::getExistingDirectory(this, tr("Please enter path to save files"));
        if(mDestination.isEmpty())
        {
            finish();
            return;
        }
    }

    const Document doc = docs.takeFirst();

    qint32 size = doc.size();
    qint32 dcId = doc.dcId();
    QString fileName = QString::number(doc.id());
    InputFileLocation input(InputFileLocation::typeInputFileLocation);
    input.setClassType(InputFileLocation::typeInputDocumentFileLocation);
    input.setId(doc.id());
    input.setAccessHash(doc.accessHash());

    ui->progressLabel->setText( QString("There are %1 media to download")
                                .arg(docs.count()) );

    QString relativeFile = fileName;
    const QString &filePath = mDestination + "/" + relativeFile;

    QDir().mkpath(mDestination);
    const QString &existingFile = fileExists(filePath);
    if(!existingFile.isEmpty())
    {
        ui->progressLabel->setText( QString("\"%1\" already downloaded.").arg(existingFile));
        downloadDocuments(docs, offset+1);
        return;
    }

    QFile *file = new QFile(filePath, this);
    if(!file->open(QFile::WriteOnly))
    {
        delete file;
        downloadDocuments(docs, offset+1);
        return;
    }

    if(!mFilesTimeoutCount.contains(doc.id()))
        mFilesTimeoutCount[doc.id()] = 0;

    const int currentTry = mFilesTimeoutCount.value(doc.id());

    QTimer *timer = new QTimer(this);
    timer->setInterval(60000);
    timer->setSingleShot(true);

    mTg->uploadGetFile(input, size, dcId, [=](TG_UPLOAD_GET_FILE_CUSTOM_CALLBACK){
        Q_UNUSED(msgId)
        if(!mFilesTimeoutCount.contains(doc.id()) ||
            mFilesTimeoutCount.value(doc.id()) != currentTry)
            return;
        if(!error.null) {
            mFilesTimeoutCount.remove(doc.id());
            QTimer::singleShot(50, this, [=](){
                downloadDocuments(docs, offset+1);
            });
            file->close();
            file->remove();
            file->deleteLater();
            timer->deleteLater();
            return;
        }

        mTotalDownloaded += result.bytes().size();
        ui->sizeLabel->setText( QString("%1KB downloaded").arg(mTotalDownloaded/1000) );

        switch ( static_cast<qint64>(result.classType()) ) {
        case UploadGetFile::typeUploadGetFileCanceled:
        case UploadGetFile::typeUploadGetFileFinished: {
            mFilesTimeoutCount.remove(doc.id());
            file->write(result.bytes());
            file->close();
            file->deleteLater();
            timer->deleteLater();

            QMimeType mime = mMimeDb.mimeTypeForFile(filePath);
            if(!mime.suffixes().isEmpty())
            {
                QString newName = filePath + "." + mime.suffixes().first();
                QFile::rename(filePath, newName);
                ui->progressLabel->setText( QString("\"%1\" downloaded successfully.")
                                            .arg(fileName + "." + mime.suffixes().first()));
            }
            else
            {
                ui->progressLabel->setText( QString("\"%1\" downloaded successfully.")
                                            .arg(fileName));
            }

            QTimer::singleShot(50, this, [=](){
                downloadDocuments(docs, offset+1);
            });
        }
            break;

        case UploadGetFile::typeUploadGetFileProgress:
            file->write(result.bytes());
            timer->stop();
            timer->start();
            break;
        }
    });

    connect(timer, &QTimer::timeout, this, [=](){
        if(!mFilesTimeoutCount.contains(doc.id()))
            return;
        file->close();
        file->remove();
        delete file;

        const int retriesLimit = 3;

        mFilesTimeoutCount[doc.id()]++;
        if(mFilesTimeoutCount.value(doc.id()) >= retriesLimit)
        {
            ui->progressLabel->setText( QString("Error: Failed to download \"%1\"").arg(fileName) );
            mFilesTimeoutCount.remove(doc.id());
            downloadDocuments(docs);
        }
        else
        {
            ui->progressLabel->setText( "Timed out. Try again..." );
            downloadDocuments(docs);
        }
    });
    timer->start();

    ui->progressBar->setValue( 1000*offset / (offset + docs.length()));
}

QString MainWindow::fileExists(const QString &path)
{
    const QString &dir = path.left(path.lastIndexOf("/"));
    QString fileName = path.mid(dir.length()+1);

    int lastIdx = fileName.lastIndexOf(".");
    if(lastIdx != -1)
        fileName = fileName.left(fileName.lastIndexOf("."));

    const QStringList &files = QDir(dir).entryList(QDir::NoDotAndDotDot|QDir::Files|QDir::Dirs);
    for(const QString &f: files)
    {
        lastIdx = f.lastIndexOf(".");
        if(lastIdx == -1) {
            if(fileName == f)
                return f;
        } else {
            if(fileName == f.left(lastIdx))
                return f;
        }
    }
    return QString();
}

void MainWindow::waitLabelHide()
{
    ui->wait->hide();
    ui->toolBar->show();
    ui->stackedWidget->show();
    ui->buttonsWidget->show();
}

void MainWindow::waitLabelShow()
{
    ui->wait->show();
    ui->toolBar->hide();
    ui->stackedWidget->hide();
    ui->buttonsWidget->hide();
}

QVariantList MainWindow::convertToList() const
{
    QVariantList res;

    QMapIterator<qint32, QMap<QDate, QMap<qint32, Message> > > ci(mMessages);
    while(ci.hasNext())
    {
        ci.next();

        qint32 peerId = ci.key();
        QMap<QDate, QMap<qint32, Message> > months = ci.value();

        Chat chat = (mChats.contains(peerId)? mChats.value(peerId) : mChat);
        User user = (mUsers.contains(peerId)? mUsers.value(peerId) : mUser);

        TGB_Chat tgbChat;
        tgbChat.name = (user.firstName() + " " + user.lastName()).trimmed();
        tgbChat.fromHash = QCryptographicHash::hash( QString::number(mTg->ourId()).toUtf8(), QCryptographicHash::Md5 ).toHex();
        tgbChat.toHash = QCryptographicHash::hash( (QString::number(mTg->ourId()) + ":" + QString::number(user.id()) + ":" + QString::number(user.accessHash())).toUtf8(), QCryptographicHash::Md5 ).toHex();

        QMapIterator<QDate, QMap<qint32, Message> > mi(months);
        while (mi.hasNext())
        {
            mi.next();

            QDate startDate = mi.key();
            QMap<qint32, Message> messages = mi.value();

            TGB_Month tgbMonth;

            Message prevMsg;
            QMapIterator<qint32, Message> msgI(messages);
            while (msgI.hasNext())
            {
                msgI.next();

                Message msg = msgI.value();
                if (msg.out())
                    insertMonth(tgbMonth.out, msg, prevMsg);
                else
                    insertMonth(tgbMonth.in, msg, prevMsg);
                insertMonth(tgbMonth.sum, msg, prevMsg);

                prevMsg = msg;
            }

            tgbChat.months[startDate] = tgbMonth;
        }

        res << tgbChat.toMap();
    }


    return res;
}

void MainWindow::insertMonth(TGB_Peer &tgb, const Message &msg, const Message &prev) const
{
    qint32 msgDelay = prev.id()? msg.date() - prev.date() : 0;
    qint32 sessionDelay = (prev.id() && prev.out() != msg.out()? msg.date() - prev.date() : -1);
    QString messageText = msg.media().caption().count()? msg.media().caption() : msg.message();

    tgb.count += 1;
    tgb.charCount += messageText.count();

    tgb.delay += msgDelay;
    if (msgDelay < 60*60)
        tgb.smallDelay += msgDelay;
    else
        tgb.longDelay += msgDelay;

    if (sessionDelay > -1)
    {
        tgb.messageSessionsCount += 1;
        tgb.delayPerSession += sessionDelay;
        if (sessionDelay < 60*60)
            tgb.smallDelayPerSession += sessionDelay;
        else
            tgb.longDelayPerSession += sessionDelay;

        tgb.maxDelayPerSession = qMax(sessionDelay, tgb.maxDelayPerSession);
        tgb.minDelayPerSession = qMax(sessionDelay, tgb.minDelayPerSession);

        tgb.avgDelayPerSession = 1.0 * tgb.delayPerSession / tgb.messageSessionsCount;
        tgb.avgSmallDelayPerSession = 1.0 * tgb.smallDelayPerSession / tgb.messageSessionsCount;
        tgb.avgLongDelayPerSession = 1.0 * tgb.longDelayPerSession / tgb.messageSessionsCount;
    }

    if (msg.media().classType() == MessageMedia::typeMessageMediaDocument)
        for (const DocumentAttribute &attr: msg.media().document().attributes())
        {
            if (attr.classType() == DocumentAttribute::typeDocumentAttributeSticker)
            {
                tgb.stickers += 1;
                QString alt = attr.alt();
                for( int i=0; i<alt.size(); i++ )
                {
                    for( int j=5; j>=0; j-- )
                    {
                        QString emoji = alt.mid(i,j);
                        if( !mEmojis.contains(emoji) )
                            continue;

                        tgb.stickerPerChar[emoji] += 1;
                        break;
                    }
                }
                break;
            }
            else
            if (attr.voice())
            {
                tgb.voiceCount += 1;
                tgb.voiceDuration += attr.duration();
                break;
            }
            else
            if (attr.roundMessage())
            {
                tgb.roundCount += 1;
                tgb.roundDuration += attr.duration();
                break;
            }
        }
    if (msg.media().classType() == MessageMedia::typeMessageMediaPhoto)
        tgb.imageCount += 1;

    bool containsEmoji = false;
    for( int i=0; i<messageText.size(); i++ )
    {
        for( int j=5; j>=0; j-- )
        {
            QString emoji = messageText.mid(i,j);
            if( !mEmojis.contains(emoji) )
                continue;

            tgb.emojisCount += 1;
            tgb.usedEmojis[emoji] += 1;
            containsEmoji = true;
            break;
        }
    }

    tgb.emojiMessagesCount += containsEmoji? 1 : 0;

    tgb.maxDelay = qMax(msgDelay, tgb.maxDelay);
    tgb.minDelay = qMin(msgDelay, tgb.minDelay);

    tgb.percentContaintsEmoji = 100.0 * tgb.emojiMessagesCount / tgb.count;
    tgb.percentVoice = 100.0 * tgb.voiceCount / tgb.count;
    tgb.percentRound = 100.0 * tgb.roundCount / tgb.count;

    tgb.avgCharCount = 1.0 * tgb.charCount / tgb.count;
    tgb.avgDelay = 1.0 * tgb.delay / tgb.count;
    tgb.avgSmallDelay = 1.0 * tgb.smallDelay / tgb.count;
    tgb.avgLongDelay = 1.0 * tgb.longDelay / tgb.count;
    tgb.avgContiniusMessages = 1.0 * tgb.count / tgb.messageSessionsCount;
    tgb.avgEmojiPerMessage = 1.0 * tgb.emojisCount / tgb.count;
    tgb.avgVoiceDuration = 1.0 * tgb.voiceDuration / tgb.voiceCount;
    tgb.avgRoundDuration = 1.0 * tgb.roundDuration / tgb.roundCount;
}

void MainWindow::finish()
{
    ui->stackedWidget->setCurrentIndex(6);

    if(mMessages.count() && ui->diaryCheck->isChecked())
    {
        QString path = mDestination + "/diary.json";
        QFile file(path);
        file.open(QFile::WriteOnly);
        file.write( QJsonDocument::fromVariant( convertToList() ).toJson() );
        file.close();
    }
    if(mMessagesList.count() && ui->messagesCheck->isChecked())
    {
        QString path = mDestination + "/messages.json";
        QFile file(path);
        file.open(QFile::WriteOnly);
        file.write( QJsonDocument::fromVariant(mMessagesList).toJson() );
        file.close();
    }
}

void MainWindow::initEmojis()
{
    if (mEmojis.count())
        return;

    QString conf = ":/files/theme";

    QFile cfile(conf);
    if ( !cfile.open(QFile::ReadOnly) )
        return;

   mEmojis.clear();

    const QString data = cfile.readAll();
    const QStringList & list = data.split("\n",QString::SkipEmptyParts);
    for ( const QString & l: list)
    {
        const QStringList & parts = l.split("\t",QString::SkipEmptyParts);
        if ( parts.count() < 2 )
            continue;

        QString epath = parts.at(0).trimmed();
        QString ecode = parts.at(1).trimmed();

        mEmojis[ecode] = epath;
    }
}

QString MainWindow::homePath()
{
    if(aseman_app_home_path)
        return *aseman_app_home_path;

    aseman_app_home_path = new QString();

    QString oldPath;
#ifdef Q_OS_WIN
    oldPath = QDir::homePath() + "/AppData/Local/" + QCoreApplication::applicationName();
#else
    oldPath = QDir::homePath() + "/.config/" + QCoreApplication::applicationName();
#endif

    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::QStandardPaths::AppDataLocation);
    if(paths.isEmpty())
        paths << oldPath;

    if( oldPath.count() && QFileInfo::exists(oldPath) )
        *aseman_app_home_path = oldPath;
    else
        *aseman_app_home_path = paths.first();

    QDir().mkpath(*aseman_app_home_path);
    return *aseman_app_home_path;
}

MainWindow::~MainWindow()
{
    delete ui;
}
