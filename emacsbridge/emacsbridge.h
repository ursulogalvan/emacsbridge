/**
 * @file emacsbridge.h
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

#ifndef EMACSBRIDGE_H
#define EMACSBRIDGE_H

#ifndef __ANDROID_API__
#include <QSystemTrayIcon>
#endif

#include <QObject>
#include <QProcess>
#include <QQmlApplicationEngine>

#include "emacsbridgetypes.h"
#include "rep_emacsbridgeremote_replica.h"

class EmacsBridge: public QObject{
    Q_OBJECT
    Q_PROPERTY(bool mobile READ mobile CONSTANT)
    Q_PROPERTY(QDateTime serviceStartupTime READ serviceStartupTime CONSTANT)
    Q_PROPERTY(QDateTime startupTime READ startupTime CONSTANT)
    Q_PROPERTY(QString defaultPage READ defaultPage WRITE setDefaultPage NOTIFY defaultPageChanged)
  public:
    explicit EmacsBridge(QObject *parent=0);
    explicit EmacsBridge(const QString &dummy, QObject *parent=0){m_isDummy=true;};
    ~EmacsBridge();

    Q_INVOKABLE void runQuery(const QString &queryKey, const QString &query);
    Q_INVOKABLE void initDrawer();
    QDateTime serviceStartupTime() const;
    QDateTime startupTime() const;
    QString defaultPage() const;
    void setDefaultPage(const QString &defaultPage);
    bool mobile() const{
#ifdef __ANDROID_API__
      return true;
#else
      return false;
#endif
    };

  signals:
    void queryFinished(const QString &queryKey, const QString &queryResult);
    void queryError(const QString &queryKey, const QString &errorMessage);
    void componentAdded(const QmlFileContainer &qmlFile);
    void componentRemoved(const QString &qmlFile);
    void dataSet(const QString &requesterId, const QJSValue &jsonData);
    void defaultPageChanged();

  private slots:
#ifndef __ANDROID_API__
    void updateNotification(const QString &title, const QString &message);
#endif
    void updateQueryResult(const QString &queryKey, const QString &result);
    void addComponent(const QmlFileContainer &qmlFile);
    void removeComponent(const QString &qmlFile);
    void setData(const JsonDataContainer &jsonContainer);

  private:
    QDateTime m_startupTime;
#ifndef __ANDROID_API__
    QSystemTrayIcon *m_trayIcon;
#endif
    QRemoteObjectNode m_repNode;
    QSharedPointer<EmacsBridgeRemoteReplica> m_rep;
    QQmlApplicationEngine *m_engine;
    bool m_isDummy=false;
    QStringList m_drawerComponents;
};

#endif
