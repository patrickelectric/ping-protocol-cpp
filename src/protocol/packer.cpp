#include <QByteArray>
#include <QDebug>
#include <QVariant>

#include "message.h"
#include "packer.h"

Packer::Packer()
{
    qDebug() << "Packer in !";
}

Packer::~Packer()
{
    qDebug() << "Packer out !";
}


// TODO: Do this in protocol compilation time
QString Packer::checkPackString(const QString& packString)
{
    if(packString[0] != '<' && packString[0] != '>') {
        qDebug() << "packString need to start with > or < to specify endian.";
        return QString();
    }
    QString formatString = packString;
    for(int i(0); i < formatString.length(); i++) {
        if(formatString[i].isDigit()) {
            int digit = formatString[i].digitValue();
            QString format(formatString[i+1]);
            formatString.replace(formatString.mid(i, 2), format.repeated(digit));
        }
    }
    return formatString;
}

QByteArray Packer::messagePack(const QVariant& messageID, const QVariant& var)
{
    return pack(Message::packString(messageID), var);
}

QByteArray Packer::messagePack(const QVariant& messageID, const QVariantList& var)
{
    return pack(Message::packString(messageID), var);
}

QByteArray Packer::pack(const QByteArray& packString, const QVariant& var)
{
    QVariantList list;
    list.append(var);
    return pack(packString, list);
}

QByteArray Packer::pack(const QByteArray& packString, const QVariantList& varList)
{
    QString formatString = checkPackString(packString);
    if(formatString.isEmpty()) {
        qDebug() << "formatString cannot be empty !";
        return QByteArray();
    } else if(formatString.length()-1 != varList.length()) {
        qDebug() << "packString and varList differ in size";
        qDebug() << formatString.length()-1 << varList.length();
    }

    QByteArray data;

    // TODO: work with endian
    for(int index(1); index < formatString.length(); index++) {
        QChar format = formatString[index];
        QVariant var = varList[index-1];
        data.append(convert(var, format));
    }

    return data;
}

QByteArray Packer::convert(const QVariant& var, const QChar& format)
{
    uint8_t uint8Value;
    uint16_t uint16Value;
    uint32_t uint32Value;
    uint64_t uint64Value;
    float floatValue;
    double doubleValue;
    switch(format.toLatin1()) {
        case('c'):
        case('b'):
        case('B'):
            uint8Value = var.toUInt();
            return QByteArray(reinterpret_cast<const char*>(&uint8Value), sizeof(uint8Value));
        case('h'):
        case('H'):
            uint16Value = var.toUInt();
            return QByteArray(reinterpret_cast<const char*>(&uint16Value), sizeof(uint16Value));
        case('i'):
        case('I'):
        case('l'):
        case('L'):
            uint32Value = var.toUInt();
            return QByteArray(reinterpret_cast<const char*>(&uint32Value), sizeof(uint32Value));
        case('q'):
        case('Q'):
            uint64Value = var.toUInt();
            return QByteArray(reinterpret_cast<const char*>(&uint64Value), sizeof(uint64Value));
        case('f'):
            floatValue = var.toFloat();
            return QByteArray(reinterpret_cast<const char*>(&floatValue), sizeof(floatValue));
        case('d'):
            doubleValue = var.toDouble();
            return QByteArray(reinterpret_cast<const char*>(&doubleValue), sizeof(doubleValue));
        default:
            break;
    }
    return QByteArray();
}

QByteArray Packer::merge(const QByteArray& header, const QByteArray& payload)
{
    QByteArray data;
    uint16_t checksum = 0;
    for(const auto& value : header + payload)
        checksum += (uint8_t) value;

    data.append(header);
    data.append(payload);
    data.append(pack(Message::checksumPackString(), checksum));
    return data;
}

QByteArray Packer::populateHeader(int messageID, int srcDevID, int dstDevID, int payload)
{
    QByteArray headerPack = Message::headerPackString();
    QVariantList list;
    list.append('B');
    list.append('R');
    list.append(payload);
    list.append(messageID);
    list.append(srcDevID);
    list.append(dstDevID);

    return pack(headerPack, list);
}

QByteArray Packer::request(const QVariant& messageID, int srcDevID, int dstDevID)
{
    QByteArray headerPack = populateHeader(Message::GeneralMessageID::gen_cmd_request, srcDevID, dstDevID, 2);
    QByteArray payloadPack = messagePack(Message::GeneralMessageID::gen_cmd_request, messageID);
    return merge(headerPack, payloadPack);
}