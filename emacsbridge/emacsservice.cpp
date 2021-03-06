/**
 * @file emacsservice.cpp
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

#ifdef __ANDROID_API__
#include <QtAndroid>
#endif

#include "emacsservice.h"
#include "emacsbridgesettings.h"

EmacsService::EmacsService(): QObject(){
  EmacsBridgeSettings *settings=EmacsBridgeSettings::instance();

  m_startupTime=QDateTime::currentDateTime();
  m_remote.setStartupTime(m_startupTime);

  m_srcNode.setHostUrl(QUrl(QStringLiteral("local:replica")));
  m_srcNode.enableRemoting(&m_remote);

  m_client=EmacsClient::instance();

  qDebug()<<"Service" << QThread::currentThreadId();

  m_server=new EmacsServer();
  m_server->moveToThread(&serverThread);

  connect(settings, SIGNAL(settingChanged(QString)), this, SLOT(settingChanged(QString)));

  // on Android the notification needs to come from the service, while on PC
  // it comes from the GUI process, making this a bit complicated.
#ifdef __ANDROID_API__
  connect(m_server, SIGNAL(notificationAdded(QString, QString)),
          this, SLOT(displayNotification(QString, QString)));
#else
  connect(m_server, SIGNAL(notificationAdded(QString, QString)),
          &m_remote, SLOT(displayNotification(QString, QString)));
#endif

  connect(m_server, SIGNAL(componentAdded(QmlFileContainer)),
          &m_remote, SLOT(addComponent(QmlFileContainer)));
  connect(m_server, SIGNAL(componentRemoved(QString)),
          &m_remote, SLOT(removeComponent(QString)));

  connect(m_server, SIGNAL(dataSet(JsonDataContainer)),
          &m_remote, SLOT(setData(JsonDataContainer)));

  connect(this, SIGNAL(startServer()),
          m_server, SLOT(startServer()));

  serverThread.start();
  emit startServer();
}

EmacsService::~EmacsService(){
}

void EmacsService::displayNotification(const QString &title, const QString &message){
#ifdef __ANDROID_API__
  QAndroidJniObject javaNotification=QAndroidJniObject::fromString(message);
  QAndroidJniObject jNotificationTitle=QAndroidJniObject::fromString(title);
  QAndroidJniObject::callStaticMethod<void>(
    "fi/aardsoft/emacsbridge/EmacsBridgeNotification",
    "notify",
    "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;)V",
    QtAndroid::androidContext().object(),
    javaNotification.object<jstring>(),
    jNotificationTitle.object<jstring>());
#endif
}

void EmacsService::settingChanged(const QString &key){
  EmacsBridgeSettings *settings=EmacsBridgeSettings::instance();

  qDebug() << "Setting changed signal for " << key;
  if (key.startsWith("localSocket/") || key.startsWith("networkSocket/")){
    if (m_client->isSetup()){
      settings->setValue("core/configured", true);
    }
  }
}
