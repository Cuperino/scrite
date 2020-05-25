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

#ifndef MODIFIABLE_H
#define MODIFIABLE_H

class Modifiable
{
public:
    void markAsModified() { ++m_modificationTime; }
    int modificationTime() { return m_modificationTime; }
    bool isModified(int *time) const {
        const bool ret = m_modificationTime > *time;
        *time = m_modificationTime;
        return ret;
    }
    bool isModified(const int time) const { return m_modificationTime > time; }

private:
    int m_modificationTime;
};

#endif // MODIFIABLE_H