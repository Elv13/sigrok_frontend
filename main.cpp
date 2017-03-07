#include <QApplication>
#include <QGuiApplication>
#include <QCommandLineParser>
#include <stdlib.h>
#include <KAboutData>
// #include <KLocalizedString>

#include "mainwindow.h"

#include <QStyleFactory>

#include <QtQuick/QQuickWindow>

#ifdef PACKED_ICONS
 #include "qrc_icons.cpp"
#endif

int main (int argc, char *argv[])
{
    // As of Qt5.8.1, it crash when using EGL
#ifdef PACKED_ICONS
    setenv("QMLSCENE_DEVICE", "softwarecontext", 1);
    QQuickWindow::setSceneGraphBackend(QSGRendererInterface::Software);
#endif

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication app(argc, argv);

    if (QStyleFactory::keys().indexOf("Android") != -1)
        app.setStyle(QStyleFactory::create("Android"));

//     KLocalizedString::setApplicationDomain("org.sigrok");

    KAboutData aboutData(
                         // The program name used internally. (componentName)
                         QStringLiteral("Captivate"),
                         // A displayable program name string. (displayName)
                         QStringLiteral("Tutorial 4"),
                         // The program version string. (version)
                         QStringLiteral("1.0"),
                         // Short description of what the app does. (shortDescription)
                         QStringLiteral("A simple text area which can load and save."),
                         // The license this code is released under
                         KAboutLicense::Custom,
                         // Copyright Statement (copyrightStatement = QString())
                         QStringLiteral("(c) 2016"),
                         // Optional text shown in the About box.
                         // Can contain any information desired. (otherText)
                         QStringLiteral("Some text..."),
                         // The program homepage string. (homePageAddress = QString())
                         QStringLiteral("http://example.com/"),
                         // The bug report email address
                         // (bugsEmailAddress = QLatin1String("submit@bugs.kde.org")
                         QStringLiteral("submit@bugs.kde.org"));
    aboutData.addAuthor(QStringLiteral("Name"), QStringLiteral("Task"), QStringLiteral("your@email.com"),
                        QStringLiteral("http://your.website.com"), QStringLiteral("OSC Username"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    MainWindow* window = new MainWindow();
    window->show();

    int ret = app.exec();

    delete window;

    return ret;
}
