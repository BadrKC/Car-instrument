#include "CANBusReader.h"
#include <QDebug>

CANBusReader::CANBusReader(QString interface, QObject *parent)
    : QObject{parent},
    m_canDevice{nullptr},
    FRAME_ID_SPEED{0x21},
    CAN_BUS_PLUGIN{"socketcan"}
{
    if (QCanBus::instance()->plugins().contains(CAN_BUS_PLUGIN)) {
        QString errorString;
        m_canDevice = QCanBus::instance()->createDevice(
            CAN_BUS_PLUGIN, interface, &errorString);
        if (!m_canDevice) {
            qDebug() << "couldn't create device";
            // TODO: Notify of the error
            qDebug() << errorString;
            return;
        } else {
            qDebug() << "device was created";
            const auto pp = m_canDevice->state();
            qDebug() << pp << " first";
            connect(m_canDevice, &QCanBusDevice::framesReceived, this, &CANBusReader::readCanData);
            pp = m_canDevice->state();
            qDebug() << pp << " second";
            connect(m_canDevice, &QCanBusDevice::errorOccurred, this, &MainWindow::processErrors);
            connect(m_canDevice, &QCanBusDevice::framesReceived, this, &MainWindow::processReceivedFrames);
            connect(m_canDevice, &QCanBusDevice::framesWritten, this, &MainWindow::processFramesWritten);
            pp = m_canDevice->state();
            qDebug() << pp << " third";
        }

        QString errorString0;
        const QList<QCanBusDeviceInfo> devices = QCanBus::instance()->availableDevices(
            QStringLiteral("socketcan"), &errorString0);
        if (!errorString0.isEmpty())
            qDebug() << errorString0;
        else {
            qDebug() << "Available interfaces";
            foreach(auto &x, devices) {
                qDebug()<< "name: " << x.name();
                qDebug()<< "channel(): " << x.channel();
                qDebug()<< "serialNumber() " << x.serialNumber();
            }
        }

        // Connect can bus to interface
        if (!m_canDevice->connectDevice()) {
            qDebug() << "connection failed";
            qDebug() << errorString;
            delete m_canDevice;
            m_canDevice = nullptr;
            return;
        }
        qDebug() << "connection was successful";
    }
}

CANBusReader::~CANBusReader() {
    if (m_canDevice) {
        m_canDevice->disconnectDevice();
        delete m_canDevice;
    }
}

void CANBusReader::readCanData()
{
    qDebug() << "read can data called";
    while (m_canDevice->framesAvailable()) {
        qDebug() << "frame available";
        QCanBusFrame frame = m_canDevice->readFrame();
        if (frame.isValid() && frame.frameId() == FRAME_ID_SPEED) {
            int speed = static_cast<int>(frame.payload().at(0));
            emit newData(speed);
        }
    }
}
