/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Copyright (C) 2022 Rochus Keller (me@rochus-keller.ch) for LeanQt
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QFUTUREWATCHER_H
#define QFUTUREWATCHER_H

#include <QtCore/qfuture.h>

#ifndef QT_NO_QFUTURE

#ifndef QT_NO_QOBJECT
#include <QtCore/qobject.h>
#else
#include "NoObject.h"
#endif

QT_BEGIN_NAMESPACE


class QEvent;

class QFutureWatcherBasePrivate;
class Q_CORE_EXPORT QFutureWatcherBase : public QObject
{
#ifndef QT_NO_QOBJECT
    Q_OBJECT
#endif
    Q_DECLARE_PRIVATE(QFutureWatcherBase)

public:
    explicit QFutureWatcherBase(QObject *parent = Q_NULLPTR);
    // de-inline dtor

    int progressValue() const;
    int progressMinimum() const;
    int progressMaximum() const;
    QString progressText() const;

    bool isStarted() const;
    bool isFinished() const;
    bool isRunning() const;
    bool isCanceled() const;
    bool isPaused() const;

    void waitForFinished();

    void setPendingResultsLimit(int limit);

#ifndef QT_NO_QOBJECT
    bool event(QEvent *event) Q_DECL_OVERRIDE;
#endif

Q_SIGNALS:
    void started();
    void finished();
    void canceled();
    void paused();
    void resumed();
    void resultReadyAt(int resultIndex);
    void resultsReadyAt(int beginIndex, int endIndex);
    void progressRangeChanged(int minimum, int maximum);
    void progressValueChanged(int progressValue);
    void progressTextChanged(const QString &progressText);

public Q_SLOTS:
    void cancel();
    void setPaused(bool paused);
    void pause();
    void resume();
    void togglePaused();

protected:
    void connectNotify (const QMetaMethod &signal) Q_DECL_OVERRIDE;
    void disconnectNotify (const QMetaMethod &signal) Q_DECL_OVERRIDE;

    // called from setFuture() implemented in template sub-classes
    void connectOutputInterface();
    void disconnectOutputInterface(bool pendingAssignment = false);

private:
    // implemented in the template sub-classes
    virtual const QFutureInterfaceBase &futureInterface() const = 0;
    virtual QFutureInterfaceBase &futureInterface() = 0;
};

template <typename T>
class QFutureWatcher : public QFutureWatcherBase
{
public:
    explicit QFutureWatcher(QObject *_parent = 0)
        : QFutureWatcherBase(_parent)
    { }
    ~QFutureWatcher()
    { disconnectOutputInterface(); }

    void setFuture(const QFuture<T> &future);
    QFuture<T> future() const
    { return m_future; }

    T result() const { return m_future.result(); }
    T resultAt(int index) const { return m_future.resultAt(index); }

#ifdef Q_QDOC
    int progressValue() const;
    int progressMinimum() const;
    int progressMaximum() const;
    QString progressText() const;

    bool isStarted() const;
    bool isFinished() const;
    bool isRunning() const;
    bool isCanceled() const;
    bool isPaused() const;

    void waitForFinished();

    void setPendingResultsLimit(int limit);

Q_SIGNALS:
    void started();
    void finished();
    void canceled();
    void paused();
    void resumed();
    void resultReadyAt(int resultIndex);
    void resultsReadyAt(int beginIndex, int endIndex);
    void progressRangeChanged(int minimum, int maximum);
    void progressValueChanged(int progressValue);
    void progressTextChanged(const QString &progressText);

public Q_SLOTS:
    void cancel();
    void setPaused(bool paused);
    void pause();
    void resume();
    void togglePaused();
#endif

private:
    QFuture<T> m_future;
    const QFutureInterfaceBase &futureInterface() const Q_DECL_OVERRIDE { return m_future.d; }
    QFutureInterfaceBase &futureInterface() Q_DECL_OVERRIDE { return m_future.d; }
};

template <typename T>
Q_INLINE_TEMPLATE void QFutureWatcher<T>::setFuture(const QFuture<T> &_future)
{
    if (_future == m_future)
        return;

    disconnectOutputInterface(true);
    m_future = _future;
    connectOutputInterface();
}

template <>
class QFutureWatcher<void> : public QFutureWatcherBase
{
public:
    explicit QFutureWatcher(QObject *_parent = Q_NULLPTR)
        : QFutureWatcherBase(_parent)
    { }
    ~QFutureWatcher()
    { disconnectOutputInterface(); }

    void setFuture(const QFuture<void> &future);
    QFuture<void> future() const
    { return m_future; }

private:
    QFuture<void> m_future;
    const QFutureInterfaceBase &futureInterface() const Q_DECL_OVERRIDE { return m_future.d; }
    QFutureInterfaceBase &futureInterface() Q_DECL_OVERRIDE { return m_future.d; }
};

Q_INLINE_TEMPLATE void QFutureWatcher<void>::setFuture(const QFuture<void> &_future)
{
    if (_future == m_future)
        return;

    disconnectOutputInterface(true);
    m_future = _future;
    connectOutputInterface();
}

QT_END_NAMESPACE
#endif // QT_NO_QFUTURE

#endif // QFUTUREWATCHER_H
