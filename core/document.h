#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QObject>
#include "tag.h"

class Document : public DocumentBase {
    Q_OBJECT

public:
    explicit Document(ProjectBase *_project, QObject *parent = 0);

public:
    QList<Tag*> tags;
    QPixmap temporaryScreenshot;

private:
    ProjectBase *project;
public:
    void createTagBasedOnPrevious(qint16 forVersion = -1);

public:
    QDomElement serialize(QDomDocument &xmlDoc);
    void deserialize(const QDomElement &xmlElement);

signals:
    
public slots:
    
};

#endif // DOCUMENT_H
