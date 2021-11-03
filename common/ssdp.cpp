#include "ssdp.h"

// qt
#include <QtCore/QDebug>
#include <QtCore/QThread>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QNetworkInterface>

// ssdp
#include "lssdp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>     // select
#include <sys/time.h>   // gettimeofday
#include "lssdp.h"

// libstdc++
#include <string>
#include <iostream>
#include <algorithm>
#include <memory>

namespace ssdp {

class Device;
class Service;

class Service : public QObject
{
    Q_OBJECT
public:
    enum Mode {
        CLIENT = 0x1,
        SERVER = 0x2,
    };

    explicit Service(const std::string& name) : m_Name(name) {
        lssdp_init(&lssdp);
        lssdp.config.ADDR_LOCALHOST = "::1";
        lssdp.config.ADDR_MULTICAST = "FF02::C";

        strncpy(lssdp.header.location.prefix,"http://\0",LSSDP_FIELD_LEN);
        strncpy(lssdp.header.location.domain,"test_location",LSSDP_FIELD_LEN);
        strncpy(lssdp.header.location.suffix,":8082\0",LSSDP_FIELD_LEN);

        strncpy(lssdp.header.search_target,"commend_switchbox\0",LSSDP_FIELD_LEN);
        strncpy(lssdp.header.device_type,"commend_switchbox\0",LSSDP_FIELD_LEN);
        strncpy(lssdp.header.sm_id,"commend_switchbox\0",LSSDP_FIELD_LEN);
        strncpy(lssdp.header.unique_service_name,"Barcode",LSSDP_FIELD_LEN);

        lssdp.port = 1900;
        lssdp.debug = true;
        lssdp.packet_received_callback  = [this](struct lssdp_ctx * lssdp, const char * packet,
          size_t packet_len) ->int{
            Q_UNUSED(lssdp)
            qDebug() << "\n\n\nNEW PACKET";
            return parse_packet(packet, packet_len);
        };
    }

    virtual ~Service() {
        lssdp_send_byebye(&lssdp);
    }

    Device* createDevice(); //TODO use shared_ptr
    Device* find_device(const std::string& barcode) const;
    bool exist(const std::string& barcode) const;
    void ping_all() const;
    void print_devices() const;

    void start() {
        lssdp_socket_create(&lssdp);

        if (m_Mode & Mode::CLIENT) {
            qDebug() << "================SEACH";
            lssdp_send_msearch(&lssdp);
        }

        if (m_Mode & Mode::SERVER) {
            qDebug() << "================NOTIFY";
            lssdp_send_notify(&lssdp);
        }
    }

    void set_prefix(const std::string& prefix);
    void set_suffix(const std::string& suffix);
    void set_domain(const std::string& domaine);

    void searchNow();

    QVector<Device*> device_list;
    QVector<Device*> paired_device_list;
        lssdp_ctx lssdp;

Q_SIGNALS:
    void deviceAdded(Device* dev);

private:
    std::string m_Name;

    std::string m_Prefix;
    std::string m_Suffix;
    std::string m_Domain;

    Mode m_Mode { (Mode) (Mode::CLIENT | Mode::SERVER)};

    int remove_device(std::string barcode);
    Device* create_remote(lssdp_packet &packet);
    int parse_packet(const char * packet,
                     size_t packet_len); //TODO make private
};

class DevicePrivate
{
public:
    Device::State state {Device::State::UNREACHABLE};
    std::string   barcode;
    std::string  location;
    long long    update_time {};
    Service* m_pSession;
};

Device::Device(Service* s) : d_ptr(new DevicePrivate)
{
    d_ptr->m_pSession = s;
}

Device::State Device::state() const
{
    return d_ptr->state;
}

QString Device::uniqueServiceName() const
{
    return d_ptr->barcode.c_str();
}

QString Device::location() const
{
    return d_ptr->location.c_str();
}

long long Device::update_time() const
{
    return d_ptr->update_time;
}

void log_callback(const char * file, const char * tag, int level, int line,
                  const char * func, const char * message);

void log_callback(const char * file, const char * tag, int level, int line,
                  const char * func, const char * message) {
    Q_UNUSED(file)
    Q_UNUSED(line)
    Q_UNUSED(func)
    std::string level_name = "DEBUG";
    if (level == LSSDP_LOG_INFO)   level_name = "INFO";
    if (level == LSSDP_LOG_WARN)   level_name = "WARN";
    if (level == LSSDP_LOG_ERROR)  level_name = "ERROR";

    qDebug() << QString("[%1][%2] %3").arg(level_name.c_str(),5).arg(tag).arg(message);
}

void print_lssdp_packet(lssdp_packet &parsed_packet);
void print_lssdp_packet(lssdp_packet &parsed_packet)
{
    qDebug() << "---------------------------------------------------";
    qDebug() << QString("METHOD: %1").arg(parsed_packet.method);
    qDebug() << QString("ST: %1").arg(parsed_packet.st);
    qDebug() << QString("USN: %1").arg(parsed_packet.usn);
    qDebug() << QString("LOCATION: %1").arg(parsed_packet.location);
    qDebug() << QString("SM_ID: %1").arg(parsed_packet.sm_id);
    qDebug() << QString("DEVICE: %1").arg(parsed_packet.device_type);
    qDebug() << QString("NTS: %1").arg(parsed_packet.nts);
    qDebug() << "---------------------------------------------------";
}

int Service::parse_packet(const char * packet,
                     size_t packet_len) {

    lssdp_packet parsed_packet = {
        {}, //st
        {}, //nts
        {}, //usn
        {}, //location
        {}, //sm_id
        {}, //device_type
        {}  //update_time
    };

    if (lssdp_packet_parser(packet, packet_len, &parsed_packet) == 0) {

        print_lssdp_packet(parsed_packet);

        qDebug() << "TYPE" << parsed_packet.nts;

        if(strcmp("ssdp:alive",parsed_packet.nts) == 0){
            create_remote(parsed_packet);
        } else if(strcmp("ssdp:byebye",parsed_packet.nts) == 0){
            remove_device(parsed_packet.usn);
        } else if (strcmp("ssdp:all",parsed_packet.nts) == 0){
        } else if (strcmp("M-SEARCH",parsed_packet.method) == 0){
            lssdp_send_notify(&lssdp);
        } else {
            qDebug() << "UNSUPPORTED PACKET" << parsed_packet.nts << parsed_packet.method;
        }

        return 0;
    } else {
        qDebug() << "Invalid PACKET !!!!";
    }

    return -1;
}

//returns 0 if device was removed
//returns -1 if no device was removed
int Service::remove_device(std::string barcode)
{

    qDebug() << "\n\nREMOVE DEVICE";

    Device* match = find_device(barcode);

    if (match){
        device_list.removeAll(match);
        print_devices();
        return 0;
    }

    return -1;

}

void Service::set_prefix(const std::string& prefix)
{
    m_Prefix = prefix;
    strncpy(lssdp.header.location.prefix,prefix.c_str(),LSSDP_FIELD_LEN);
}


void Service::set_suffix(const std::string& suffix)
{
    m_Suffix = suffix;
    strncpy(lssdp.header.location.suffix,suffix.c_str(),LSSDP_FIELD_LEN);
}


void Service::set_domain(const std::string& domain)
{
    m_Domain = domain;
    strncpy(lssdp.header.location.domain,domain.c_str(),LSSDP_FIELD_LEN);
}


//Find device in device list with matching barcode
//Returns Null if not found
Device* Service::find_device(const std::string& barcode) const
{
    for (auto dev : device_list) {
        if(dev->d_ptr->barcode == barcode){
            return dev;
        }
    }
    return NULL;
}


bool Service::exist(const std::string& barcode) const
{
    return !find_device(barcode);
}

Device* Service::create_remote(lssdp_packet &packet)
{

    const auto check_barcode = packet.usn;

    if(!exist(check_barcode))
        return nullptr;

    Device *ptr = new Device(this);
    ptr->d_ptr->barcode = packet.usn;
    ptr->d_ptr->location = packet.location;
    ptr->d_ptr->update_time=packet.update_time;
    device_list.push_back(ptr);

    Q_EMIT deviceAdded(ptr);

    print_devices();
    return ptr;
}


int Device::ping() const
{
    //TODO
    return 0;
}

void Service::ping_all() const
{
    for(const auto& dev : device_list)
        dev->ping();
}

void Service::print_devices() const
{
    qDebug() <<"--Device List---";

    for(auto dev : device_list){
        std::string status;
        if (dev->state() == Device::State::PAIRED) {
            status = "PAIRED";
        } else {
            status = "UNREACHABLE";
        }
        qDebug() <<  dev->uniqueServiceName() << "    " << status.c_str();
    }
    qDebug() <<"--End Device List---";
}

class SSDPPrivate
{
public:
    QThread* m_pThread {nullptr};
    bool m_Exit {false};
    Service* m_pService {nullptr};
    bool m_AutoBroadcast {true};
};


void Service::searchNow()
{
    lssdp_send_msearch(&lssdp);
}

void SSDP::searchNow()
{
    d_ptr->m_pService->searchNow();
}

SSDP::SSDP(QObject* parent) : QAbstractListModel(parent), d_ptr(new ssdp::SSDPPrivate)
{

    d_ptr->m_pService = new Service("captivate");
    d_ptr->m_pService->set_prefix("captivate://\0");
    d_ptr->m_pService->set_suffix(":2223");

    foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
            d_ptr->m_pService->set_domain(address.toString().toLatin1().data());
    }


    lssdp_set_log_callback(log_callback);

    d_ptr->m_pService->start();

    d_ptr->m_pThread = new QThread(this);

    static int count = 0;

    // Client
    connect(d_ptr->m_pThread, &QThread::started, [this]() {
        // Main Loop
        for (;;) {

            // quit gracefully
            if (d_ptr->m_Exit) {
                d_ptr->m_pThread->exit();
                return;
            }

            fd_set fs;
            FD_ZERO(&fs);
            FD_SET(d_ptr->m_pService->lssdp.sock, &fs);
            struct timeval tv;
            tv.tv_sec = 5;
            tv.tv_usec = 0;// 500 * 1000;   // 2s

            int ret = select(d_ptr->m_pService->lssdp.sock + 1, &fs, NULL, NULL, &tv);

            if (ret < 0) {
                sleep(2);
                qDebug() << "select error, errno = " << errno <<
                  (fcntl(d_ptr->m_pService->lssdp.sock, F_GETFD) != -1 || errno != EBADF);
            }

            if (ret > 0) {
                lssdp_socket_read(&d_ptr->m_pService->lssdp);
            }

            // Notify even if no-one ask, UDP is unreliable
            if (d_ptr->m_AutoBroadcast && count++ >= 6) {
                count = 0;
                lssdp_send_notify(&d_ptr->m_pService->lssdp);
            }
        }
    });

    connect(d_ptr->m_pService, &Service::deviceAdded, [this](Device* dev) {
        const int len = d_ptr->m_pService->device_list.length() - 1;
        beginInsertRows({}, len, len);
        Q_EMIT deviceAdded(dev);
        endInsertRows();
        qDebug() << "==============INSERT" << len << dev->location() << index(0,0).data();
        Q_EMIT dataChanged(index(len, 0),index(len, 0));
    });

    d_ptr->m_pThread->start();

}

SSDP::~SSDP()
{
    d_ptr->m_Exit = true;
    d_ptr->m_pThread->wait();
    delete d_ptr;
}


QVariant SSDP::data(const QModelIndex& idx, int role) const
{
    qDebug() << "////////////";
    if (!idx.isValid())
        return {};

    const auto dev = d_ptr->m_pService->device_list[idx.row()];

    qDebug() << "DATA" << dev->location();
    switch(role) {
        case Qt::DisplayRole:
            return dev->location();
        case Role::LOCATION:
            return dev->location();
        case Role::PORT:
            return "foo";
        case Role::IP:
            return "bar";
    }

    return {};
}

int SSDP::rowCount(const QModelIndex& parent) const
{
    qDebug() << "9999999999999999999999999ROWCOUNT" << parent.isValid() << d_ptr->m_pService->device_list.count();
    return parent.isValid() ? 0 : d_ptr->m_pService->device_list.count();
}

QHash<int, QByteArray> SSDP::roleNames() const
{
    static QHash<int, QByteArray> ret;

    if (ret.isEmpty()) {
        ret[Role::LOCATION] = "location";
        ret[Role::PORT    ] = "port";
        ret[Role::IP      ] = "ip";
    }

    return {};
}

void SSDP::setAutoBroadcast(bool value)
{
    d_ptr->m_AutoBroadcast = value;
    Q_EMIT autoBroadcastChanged(value);
}

bool SSDP::isAutoBroadcast() const
{
    return d_ptr->m_AutoBroadcast;
}

Device* SSDP::deviceAt(const QModelIndex& idx) const
{
    if (!idx.isValid())
        return nullptr;

    return d_ptr->m_pService->device_list[idx.row()];
}

} //namespace

#include <ssdp.moc>
