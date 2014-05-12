#include "displayvacationcheckedpathwindow.h"
#include "ui_displayvacationcheckedpathwindow.h"
#include "QDebug"
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QString>
#include <QtXml/QDomDocument>
#include <QtXml/QDomDocument>

displayVacationCheckedPathWindow::displayVacationCheckedPathWindow(QList<QString> checkedBoxList, QString startLocation, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::displayVacationCheckedPathWindow)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    QString distanceURL;

    this->startLocation = startLocation;

    for (int var = 0; var < checkedBoxList.count(); ++var) {
        query.exec("SELECT address, stadiumTeamName FROM stadiums WHERE stadiumTeamName = \"" + checkedBoxList.at(var).split(",")[0] + "\";");
        while(query.next())
        {
            checkedBoxAddress.append(query.value(0).toString());
        }
    }

    query.exec("SELECT address FROM stadiums WHERE stadiumTeamName = \"" + startLocation.split(",")[0] + "\";");
    while(query.next())
    {
        startLocationAddress = query.value(0).toString();
    }

    distanceURL = "http://maps.googleapis.com/maps/api/distancematrix/xml?";

    distanceURL += "origins=" + startLocationAddress.replace(", ", "+").replace(" ", "+") + "&destinations=" + checkedBoxAddress.at(0).toUtf8().replace(", ", "+").replace(" ", "+");
    for (int var = 1; var < checkedBoxAddress.count(); ++var) {
          distanceURL += "|" + checkedBoxAddress.at(var).toUtf8().replace(", ", "+");
    }

    distanceURL += "&mode=driving&sensor=false&units=imperial";

    QNetworkAccessManager * manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(fileIsReady(QNetworkReply*)) );
    manager->get(QNetworkRequest(QUrl(distanceURL)));

}


void displayVacationCheckedPathWindow::fileIsReady( QNetworkReply * reply)
{
    hashedList.clear();
    QString mapURL = "http://maps.googleapis.com/maps/api/staticmap?size=640x640&sensor=false&maptype=hybrid&path=color:0x0000ff80|weight:5|" + startLocationAddress;
    QString returnedXML = reply->readAll();
    QString outputText;
    int totalDistance = 0;

    QDomDocument mDocument;
    QDomElement  mDocumentElement;

    mDocument.setContent(returnedXML);

    QDomNode row = mDocument.namedItem("DistanceMatrixResponse").lastChildElement();
    QDomNode root = mDocument.namedItem("DistanceMatrixResponse");
    QDomNodeList distanceNode = row.childNodes();
    QDomNodeList nameNode = root.childNodes();

    for (int i = 0; i < distanceNode.count(); ++i)
    {
        hashedList.insert(distanceNode.at(i).toElement().namedItem("distance").childNodes().at(1).toElement().text().replace(",","").split(" ")[0].toDouble(), nameNode.at(i+2).toElement().text());
    }

    QMap<double, QString>::iterator i;
    // Add Start Location
    mapURL += "|" + startLocationAddress;
    for (i = hashedList.begin(); i != hashedList.end(); ++i)
    {
         mapURL += "|" + i.value().replace(", ", "+").replace(" ", "+");
    }
    // Return to start
    mapURL += "|" + startLocationAddress;

    mapURL  += "&markers=color:red%7Clabel+" + startLocationAddress;

    for (i = hashedList.begin(); i != hashedList.end(); ++i)
    {
        mapURL += "|" + i.value().replace(", ", "+").replace(" ", "+");
    }

    outputText = "The shortest Path from " + startLocation + " to " + hashedList.end().operator --().value() + ".";

    for (i = hashedList.begin(); i != hashedList.end(); ++i)
    {
        totalDistance = i.key() - totalDistance;
        outputText += "\n" + i.value().replace("+", " ") + " Distance " + QString::number(i.key());
    }
    outputText += "\n\nTotal Distance Traveled: " + QString::number(totalDistance);
    ui->outputLabel->setText(outputText);
    ui->webDisplayBox->load(QUrl(mapURL));

}

displayVacationCheckedPathWindow::~displayVacationCheckedPathWindow()
{
    delete ui;
}
