/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "lockfile.h"

#include <QDir>
#include <QUuid>
#include <QTimer>
#include <QDateTime>
#include <QFileInfo>
#include <QJsonDocument>
#include <QCoreApplication>
#include <QFileSystemWatcher>

LockFile::LockFile(QObject *parent)
    : QObject(parent)
{
    m_modifiedTimer = new QTimer(this);
    m_modifiedTimer->setSingleShot(true);
    m_modifiedTimer->setInterval(0);

    connect(m_modifiedTimer, &QTimer::timeout, this, &LockFile::modified);

    connect(this, &LockFile::canReadChanged, m_modifiedTimer, QOverload<>::of(&QTimer::start));
    connect(this, &LockFile::canWriteChanged, m_modifiedTimer, QOverload<>::of(&QTimer::start));
    connect(this, &LockFile::lockInfoChanged, m_modifiedTimer, QOverload<>::of(&QTimer::start));
    connect(this, &LockFile::claimedChanged, m_modifiedTimer, QOverload<>::of(&QTimer::start));
    connect(this, &LockFile::filePathChanged, m_modifiedTimer, QOverload<>::of(&QTimer::start));
}

LockFile::~LockFile()
{
    this->cleanup();
}

void LockFile::setFilePath(const QString &val)
{
    if(m_filePath == val)
        return;

    this->cleanup();
    m_filePath = val;
    this->initialize();

    emit filePathChanged();
}

void LockFile::setStrategy(Strategy val)
{
    if(m_strategy == val)
        return;

    m_strategy = val;
    emit strategyChanged();

    this->updateStatus();
}

bool LockFile::claim()
{
    this->setLockInfo( QJsonObject() );
    this->updateStatus();
    return this->isClaimed();
}

void LockFile::cleanup()
{
    const QJsonObject linfo = m_lockInfo;

    this->setCanRead(false);
    this->setCanWrite(false);
    this->setLockInfo(QJsonObject());
    this->setClaimed(false);

    if(m_fsWatcher)
        delete m_fsWatcher;
    m_fsWatcher = nullptr;

    if(!m_lockFilePath.isEmpty() && QFile::exists(m_lockFilePath))
    {
        const bool hasUniqueId = !m_uniqueId.isEmpty();
        const QString ownerUniqueId = linfo.value(QStringLiteral("id")).toString();
        if(hasUniqueId && ownerUniqueId == m_uniqueId)
            QFile::remove(m_lockFilePath);
    }

    m_lockFilePath.clear();
    m_uniqueId.clear();
}

void LockFile::initialize()
{
    // The assumption is that cleanup() is called before this function is called.

    if(m_filePath.isEmpty())
        return;

    QFileInfo fi(m_filePath);
    if(!fi.exists() || !fi.isFile())
        return;

    m_uniqueId = QUuid::createUuid().toString();
    m_lockFilePath = fi.absoluteDir().absoluteFilePath(fi.fileName() + QStringLiteral(".lock"));
    this->updateStatus();
}

void LockFile::updateStatus()
{
    if(m_lockFilePath.isEmpty())
        return;

    QFileInfo fi(m_lockFilePath);
    if(!fi.exists())
    {
        if(m_lockInfo.isEmpty())
        {
            QFile file(m_lockFilePath);
            if( !file.open(QFile::WriteOnly|QFile::Unbuffered|QFile::Text) )
            {
                this->setCanRead(false);
                this->setCanWrite(false);
                this->setLockInfo(QJsonObject());
                return;
            }

            QJsonObject info;
            info.insert( QStringLiteral("name"), qApp->applicationName() );
            info.insert( QStringLiteral("pid"), qApp->applicationPid() );
            info.insert( QStringLiteral("id"), m_uniqueId );
            info.insert( QStringLiteral("version"), qApp->applicationVersion() );
            info.insert( QStringLiteral("org"), qApp->organizationName() );
            info.insert( QStringLiteral("domain"), qApp->organizationDomain() );
            info.insert( QStringLiteral("timestamp"), QDateTime::currentDateTime().toString() );

            file.write( QJsonDocument(info).toJson() );

            this->setClaimed(true);
        }
        else
            this->setClaimed(false);
    }

    QFile file(m_lockFilePath);
    if( !file.open(QFile::ReadOnly) )
    {
        this->setCanRead(false);
        this->setCanWrite(false);
        this->setLockInfo(QJsonObject());
        this->setClaimed(false);
        return;
    }

    if(m_fsWatcher == nullptr)
    {
        m_fsWatcher = new QFileSystemWatcher(this);
        m_fsWatcher->addPath(m_lockFilePath);
        connect(m_fsWatcher, &QFileSystemWatcher::fileChanged, this, &LockFile::updateStatus);
    }

    this->setLockInfo( QJsonDocument::fromJson(file.readAll()).object() );
    if(m_lockInfo.isEmpty())
    {
        this->setCanRead(false);
        this->setCanWrite(false);
        this->setClaimed(false);
        return;
    }

    const bool lockedBySomeoneElse = m_lockInfo.value( QStringLiteral("id") ).toString() != m_uniqueId;
    if(lockedBySomeoneElse)
    {
        this->setCanWrite(false);
        this->setCanRead(m_strategy == MultipleReadSingleWrite);
    }
    else
    {
        this->setCanRead(true);
        this->setCanWrite(true);
    }

    this->setClaimed(true);
}

void LockFile::setCanRead(bool val)
{
    if(m_canRead == val)
        return;

    m_canRead = val;
    emit canReadChanged();
}

void LockFile::setCanWrite(bool val)
{
    if(m_canWrite == val)
        return;

    m_canWrite = val;
    emit canWriteChanged();
}

void LockFile::setLockInfo(const QJsonObject &val)
{
    if(m_lockInfo == val)
        return;

    m_lockInfo = val;
    emit lockInfoChanged();
}

void LockFile::setClaimed(bool val)
{
    if(m_claimed == val)
        return;

    m_claimed = val;
    emit claimedChanged();

    if(!val && m_fsWatcher != nullptr)
    {
        delete m_fsWatcher;
        m_fsWatcher = nullptr;
    }
}
