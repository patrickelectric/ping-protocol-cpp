#include <QDebug>

#include "link.h"

#include "filelink.h"
#include "seriallink.h"
#include "tcplink.h"
#include "udplink.h"

Link::Link(AbstractLink::LinkType linkType, QString name)
{
    qDebug() << "Link in !" << linkType;

    switch(linkType) {
        case AbstractLink::LinkType::Serial :
            _abstractLink = new SerialLink();
            break;
        default :
            qDebug() << "Link not available!";
            return;
    }

    _abstractLink->setParent(this);
    _abstractLink->setName(name);
}

Link::~Link()
{
    qDebug() << "Link out !";
}