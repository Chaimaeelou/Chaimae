#include "watcherfeeling.h"
#include "ui_watcherfeeling.h"

WatcherFeelingWaiter::WatcherFeelingWaiter(QWidget *parent) :
    QPushButton(parent) {
    counter     = 0;
    showCounter = false;
    startTimer(1000);
}
void WatcherFeelingWaiter::resetCounter(const QString &_baseText, quint16 _counter) {
    baseText = _baseText;
    counter  = _counter;
    showCounter = true;
    timerEvent(0);
}
void WatcherFeelingWaiter::disableCounter() {
    counter  = 0;
    showCounter = false;
    timerEvent(0);
}

void WatcherFeelingWaiter::timerEvent(QTimerEvent *e) {
    if((showCounter) && (counter)) {
        if(e)
            counter--;
        if(counter == 0)
            emit(released());
        else
            setText(QString("%1 (%2)").arg(baseText).arg(counter));
    }
    else
        setText(baseText);
}


WatcherFeeling::WatcherFeeling(QWidget *parent) :
    QWidget(parent, Qt::Widget | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint),
    ui(new Ui::WatcherFeeling) {
    ui->setupUi(this);
}

void WatcherFeeling::display(Document *document) {
    bool resetInfo = false;
    if(!isVisible())
        documents.clear();
    if(!document) {
        resetInfo = true;
        ui->ok->resetCounter("Save");
    }
    else if(!documents.contains(document)) {
        documents.append(document);
        resetInfo = true;
        ui->ok->resetCounter("Ok", 5);
    }
    if(resetInfo) {
        ui->baseText->clear();
        ui->moreText->clear();
        ui->location->setText(Global::userInfos->getInfo("Location (verbose)"));
        if(!Global::userInfos->getInfo("Weather (verbose)").isEmpty()) {
            ui->weatherTemperature->setVisible(true);
            ui->weatherIcon->setVisible(true);
            ui->weatherSky->setVisible(true);
            ui->weatherTemperature->setText(QString("%1°C").arg(Global::userInfos->getInfo("Weather Temperature")));
            ui->weatherSky        ->setText(Global::userInfos->getInfo("Weather Sky"));
            ui->weatherIcon->setPixmap(QPixmap(":/weather/weather/" + Global::userInfos->getInfo("Weather Sky Icon") + ".png"));
        }
        else {
            ui->weatherTemperature->setVisible(false);
            ui->weatherIcon->setVisible(false);
            ui->weatherSky->setVisible(false);
        }
        ui->preview->setChecked(true);
        ui->preview->setIcon(QPixmap::fromImage(Global::temporaryScreenshot));
        move(QApplication::desktop()->screenGeometry().topRight() + QPoint(-width()-5, 5));
    }
    if(documents.count() == 1)       ui->label->setText(tr("Something to say about your changes on %1 since last time (%2)?").arg(documents.first()->file.baseName()).arg(Global::dateToString(documents.first()->getMetadata("Document Date/Time").toDateTime())));
    else if(documents.count()  > 1)  ui->label->setText(tr("Something to say about your changes on these %1 since last time?").arg(Global::plurial(documents.count(), "document")));
    else if(documents.count() == 0)  ui->label->setText(tr("Something to say?"));
    show();
}

WatcherFeeling::~WatcherFeeling() {
    delete ui;
}

void WatcherFeeling::changeEvent(QEvent *e) {
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    case QEvent::ActivationChange:
        ui->ok->disableCounter();
        break;
    default:
        break;
    }
}

void WatcherFeeling::action() {
    if((sender() == ui->ok) || (sender() == ui->baseText)) {
        if(documents.count() == 0) {
            Document *document = new Document(Global::currentProject);
            documents.append(document);
            if(!ui->baseText->text().isEmpty()) document->updateImport(QString("Note %1: %2").arg(Global::currentProject->noteId).arg(ui->baseText->text()));
            else                                document->updateImport(QString("Note %1").arg(Global::currentProject->noteId));
            document->setMetadata("Rekall", "Note ID", Global::currentProject->noteId, document->getMetadataIndexVersion(-1));
            document->createTagBasedOnPrevious();
            Global::currentProject->noteId++;
        }
        foreach(Document *document, documents) {
            QString action = "Creation";
            if(document->file.exists()) {
                if(document->updateFile(document->file))
                    document->createTagBasedOnPrevious();
                else
                    action = "Update";
            }
            qint16 version = document->getMetadataIndexVersion(-1);
            if(!ui->baseText->text().isEmpty())
                document->setMetadata("Rekall", "Comments", ui->baseText->text(), version);
            if(!ui->moreText->toPlainText().isEmpty())
                document->setMetadata("Rekall", "Comments (details)", ui->moreText->toPlainText(), version);
            if(!ui->location->text().isEmpty()) {
                QStringList gpsElements = ui->location->text().split("@", QString::SkipEmptyParts);
                if(gpsElements.count() > 1) {
                    document->setMetadata("Rekall", "Import Location Place", gpsElements.at(0).trimmed(), version);
                    document->setMetadata("Rekall", "Import Location GPS",   gpsElements.at(1).trimmed(), version);
                }
                else if(gpsElements.count() > 0)
                    document->setMetadata("Rekall", "Import Location Place", gpsElements.first().trimmed(), version);
            }
            if(!Global::userInfos->getInfo("Weather (verbose)").isEmpty()) {
                document->setMetadata("Rekall", "Import Weather Temperature", Global::userInfos->getInfo("Weather Temperature"), version);
                document->setMetadata("Rekall", "Import Weather Sky",         Global::userInfos->getInfo("Weather Sky"),  version);
                document->setMetadata("Rekall", "Import Weather Sky Icon",    Global::userInfos->getInfo("Weather Sky Icon"), version);
            }
            if(ui->preview->isChecked()) {
                QString filename = "";
                if(document->file.exists()) {
                    filename = QString("%1_%2.jpg").arg(Global::cacheFile("comment", document->file)).arg(version);
                    document->setMetadata("Rekall", "Snapshot", "Comment", version);
                }
                else {
                    filename = QString("%1_%2.jpg").arg(Global::cacheFile("note", document->getMetadata("Rekall", "Note ID").toString())).arg(version);
                    document->setMetadata("Rekall", "Snapshot", "Note", version);
                }
                Global::temporaryScreenshot.save(filename, "jpeg", 70);
            }

            Global::feedList->addFeed(new FeedItemBase(document->getMetadata("Document Name").toString(), action, document->getMetadata("Import User Name").toString(), document->getMetadata("Import Date/Time").toDateTime()));
        }
        Global::timelineSortChanged = Global::viewerSortChanged = Global::eventsSortChanged = Global::metaChanged = true;
        close();
    }
    else if(sender() == ui->cancel) {
        close();
    }
}
