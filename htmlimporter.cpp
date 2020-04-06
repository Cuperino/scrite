/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "htmlimporter.h"

HtmlImporter::HtmlImporter(QObject *parent)
                  :AbstractImporter(parent)
{

}

HtmlImporter::~HtmlImporter()
{

}

bool HtmlImporter::doImport(QIODevice *device)
{
    QString errMsg;
    int errLine = -1;
    int errCol = -1;

    QDomDocument htmlDoc;
    if( !htmlDoc.setContent(device, &errMsg, &errLine, &errCol) )
    {
        const QString msg = QString("Parse Error: %1 at Line %2, Column %3").arg(errMsg).arg(errLine).arg(errCol);
        this->error()->setErrorMessage(msg);
        return false;
    }

    const QDomElement rootE = htmlDoc.documentElement();
    const QDomElement bodyE = rootE.firstChildElement("body");
    if(bodyE.isNull())
    {
        this->error()->setErrorMessage("Could not find <BODY> tag.");
        return false;
    }

    const QDomNodeList pList = bodyE.elementsByTagName("p");
    if(pList.isEmpty())
    {
        this->error()->setErrorMessage("No paragraphs to import.");
        return false;
    }

    this->progress()->setProgressStep(1.0 / qreal(pList.size()+1));
    this->configureCanvas(pList.size());

    static const QStringList types = QStringList()
            << "heading" << "action" << "character"
            << "dialog" << "parenthetical" << "shot"
            << "transition";

    Scene *scene = nullptr;
    QDomElement paragraphE = bodyE.firstChildElement("p");
    while(!paragraphE.isNull())
    {
        TraverseDomElement tde(paragraphE, this->progress());

        const QString type = paragraphE.attribute("class");
        const int typeIndex = types.indexOf(type);
        if(typeIndex < 0)
            continue;

        QString text = paragraphE.text().trimmed();
        text = text.replace("\r\n", " ");
        text = text.replace("\n", " ");
        if(text.isEmpty())
            continue;

        switch(typeIndex)
        {
        case 0:
            scene = this->createScene(text);
            break;
        case 1:
            this->addSceneElement(scene, SceneElement::Action, text);
            break;
        case 2:
            this->addSceneElement(scene, SceneElement::Character, text);
            break;
        case 3:
            this->addSceneElement(scene, SceneElement::Dialogue, text);
            break;
        case 4:
            this->addSceneElement(scene, SceneElement::Parenthetical, text);
            break;
        case 5:
            this->addSceneElement(scene, SceneElement::Shot, text);
            break;
        case 6:
            this->addSceneElement(scene, SceneElement::Transition, text);
            break;
        }
    }

    return true;
}
