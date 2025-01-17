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

#ifndef APPWINDOW_H
#define APPWINDOW_H

#include <QQuickView>
#include <QQmlEngine>

class AppWindow : public QQuickView
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    static AppWindow *instance();
    AppWindow();
    ~AppWindow();

private:
    void initializeFileNameToOpen();
};

#endif // APPWINDOW_H
