/*  WikiOnBoard
    Copyright (C) 2011  Christian Puehringer

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.*/

#include <QtGui/QApplication>
#include <qdeclarative.h>
#include "qmlapplicationviewer.h"
#include "indexlistqml.h"
#include "mediakeycaptureitem.h"
#include "zimreply.h"
#include <QTranslator>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativePropertyMap>
#include <QTimer>
#include <QtScroller>
#include <QNetworkAccessManager>
#include <QDeclarativeNetworkAccessManagerFactory>
#if defined(Q_OS_ANDROID)
#include <QSystemInfo>
#ifdef QTM_NAMESPACE
QTM_USE_NAMESPACE
#endif
#endif

//Get VERSION from qmake .pro file as string
#define __VER1M__(x) #x
#define __VERM__(x) __VER1M__(x)
#define __APPVERSIONSTRING__ __VERM__(__APPVERSION__)



class ZimNetworkAccessManager : public QNetworkAccessManager
{
public:
    ZimNetworkAccessManager(QObject* parent = 0) : QNetworkAccessManager(parent)
    {
    }

    virtual QNetworkReply *createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
    {
        qDebug() << "In ZimNetworkAccessManager::createRequest. Operation : " << op << " request.url():" << request.url();

        //if (op != GetOperation || request.url().scheme() != QLatin1String("qt-render") || request.url().host() != QLatin1String("button"))
        //TODO external links?
        if (op != GetOperation )
            return QNetworkAccessManager::createRequest(op, request, outgoingData);
        return new ZimReply(this, request);
    }
};



class MyNetworkAccessManagerFactory : public QDeclarativeNetworkAccessManagerFactory
 {
 public:
     virtual QNetworkAccessManager *create(QObject *parent);
 };

QNetworkAccessManager *MyNetworkAccessManagerFactory::create(QObject *parent)
 {
     QNetworkAccessManager *nam = new ZimNetworkAccessManager(parent);
     return nam;
 }

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QTime timer;
    timer.start();
    #if defined(Q_OS_SYMBIAN) || defined(Q_WS_SIMULATOR)
    #else
      //On harmattan default rendered (should be "meego") slow for
      // articles with images. (see https://github.com/cip/WikiOnBoard/issues/72)
      // => Use raster renderer instead, which is surprisingly much faster
      QApplication::setGraphicsSystem(QLatin1String("raster"));
    #endif
    QScopedPointer<QApplication>
           app(createApplication(argc, argv));
    //Load translation
    QTranslator translatorDefault;
    QTranslator translatorLocale;

    qDebug() << "current path:" << QDir::currentPath();    
    qDebug() << "application dir path: "<<QCoreApplication::applicationDirPath();
    //translatins are stored as qt resource
    QString translationsDir =  QString(QLatin1String(":/translations"));
    qDebug() << "searching translation files in directory: "<<translationsDir;
    //First load english tranlsations. (replaces translation from source code, to separate text and code more)
    if (translatorDefault.load(QLatin1String("wikionboard_en"), translationsDir)) {
        qDebug() <<"Installing translator for english. (Replaces (some) texts from source code)";
        app->installTranslator(&translatorDefault);
    } else {
        qWarning() << "Loading translation file for english failed. Will use text from source code instead, which may be less complete/polished";
    }
#if defined(Q_OS_ANDROID)
    // QLocale::system() not working in necessitas (at least in alpha 3)-> use system info instead
    QSystemInfo *systemInfo = new QSystemInfo(0);
    QString locale = systemInfo->currentLanguage();
    qDebug() << "locale as returned by currentLanguage: "<<locale;
    //TODO remove when fixed in necessitas. Note that workaround (obviously) doesn't work for all languages
    locale.truncate(2);
    locale = locale.toLower();
    qDebug() << "Locale after hack to workaround issue that display name instead of iso code returned: "<< locale;
    delete systemInfo;
#else
    QString locale = QLocale::system().name();
#endif

    //Load tranlsation for local language, if available.
    //Note: If translator.load does not find locale file, it automatically strips local name and tries again.
    //  E.g. If wikionboard_de_AT.qsf does not exist, it tries wikionboard_de.qsf next.
    //Note: In case of english locale the same translator is installed twice. This should however
    // not have a negative impact.
    if (translatorLocale.load(QLatin1String("wikionboard_") + locale, translationsDir)) {
        qDebug() << "Installing translator for locale: "<<locale;
        app->installTranslator(&translatorLocale);
    } else {
        qDebug() << "Loading translation file for locale " << locale << " failed. (english translator is used instead)";
    }
    qDebug() << timer.elapsed() <<" ms";
    qmlRegisterType<ZimFileWrapper>("WikiOnBoardModule", 1, 0, "ZimFileWrapper");
    qmlRegisterType<IndexListQML>("WikiOnBoardModule", 1, 0, "IndexListQML");
    qDebug() << timer.elapsed() <<" ms" << "qml types registered";
    qmlRegisterType<MediakeyCaptureItem>("Mediakey", 1, 0, "MediakeyCapture");

    QmlApplicationViewer viewer;
    QDeclarativeContext *context = viewer.rootContext();
    QDeclarativePropertyMap appInfo;
    appInfo.insert(QLatin1String("version"), QVariant(QString::fromLocal8Bit(__APPVERSIONSTRING__)));
    bool isSelfSigned= false;
    #if __IS_SELFSIGNED__==1
        isSelfSigned = true;
    #endif
    appInfo.insert(QLatin1String("isSelfSigned"), QVariant(isSelfSigned));
    appInfo.insert(QLatin1String("buildDate"), QVariant(QString::fromLocal8Bit(__DATE__)));
    QString platform = QLatin1String("other");
#if defined(Q_OS_SYMBIAN) || defined(Q_WS_SIMULATOR)
        platform = QString(QLatin1String("symbian"));
    #elif defined(Q_OS_ANDROID)
        platform = QString(QLatin1String("android"));
    #endif
    appInfo.insert(QLatin1String("platform"),QVariant(platform));
    context->setContextProperty(QLatin1String("appInfo"), &appInfo);

    #ifdef Q_OS_ANDROID
        //For Qt Components for Android
        viewer.addImportPath(QLatin1String("/imports/"));
        viewer.engine()->addPluginPath(QDir::homePath()+QLatin1String("/../lib"));
    #endif
    #if defined(Q_OS_ANDROID)
        //Workaround for issue that import path incorrect
        // (e.g. /tmp/necessitas/unstable/Android/Qt/480/build-armeabi/install/imports)
        //  However,very odd that it has worked without workaround before,
        // and then it was not working in none of the tested configurations. (e.g. emulator, device
        // alpha 3 and 4, local deployment and ministro)
        viewer.engine()->addImportPath(QLatin1String("/data/data/org.kde.necessitas.ministro/files/qt/imports"));
        qDebug() << "view->engine()->importPathList()():"<<viewer.engine()->importPathList();
        qDebug() << "view->engine()->pluginPathList():"<<viewer.engine()->pluginPathList();
    #endif

    qDebug() << timer.elapsed() <<" ms " << "Application viewer created";
    viewer.engine()->setNetworkAccessManagerFactory(new MyNetworkAccessManagerFactory);
    viewer.setMainQmlFile(QLatin1String("qml/WikiOnBoardComponents/main.qml"));
    viewer.showExpanded();
    qDebug() << timer.elapsed() <<" ms " << "main qml set";
    #ifdef Q_OS_SYMBIAN
        QtScrollerProperties sp;
        sp.setScrollMetric(QtScrollerProperties::MousePressEventDelay,  qreal(0.1));
        sp.setScrollMetric(QtScrollerProperties::DragStartDistance,   qreal(2.5 / 1000) );
        QtScrollerProperties::setDefaultScrollerProperties(sp);
    #endif
    return app->exec();
}

