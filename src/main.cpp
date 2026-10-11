#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFontDatabase>
#include <QFont>
#include "controller/SessionController.h"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    app.setOrganizationName("DecisionHelper");
    app.setApplicationName("DecisionHelper");

    // ── Emoji font setup ──────────────────────────────────────────────────
    // Critical for WebAssembly which does not have access to host system fonts.
    QString emojiFamily;
#ifdef HAS_EMOJI_FONT
    const int fontId = QFontDatabase::addApplicationFont(
        QStringLiteral(":/fonts/NotoColorEmoji.ttf"));
    if (fontId == -1) {
        qWarning("Failed to load NotoColorEmoji.ttf from resources (fontId == -1)");
    } else {
        const QStringList families = QFontDatabase::applicationFontFamilies(fontId);
        if (!families.isEmpty()) {
            emojiFamily = families.first();
            qDebug("Emoji font loaded successfully: %s", qPrintable(emojiFamily));
            QFont::insertSubstitution(app.font().family(), emojiFamily);
            QFont::insertSubstitution(QStringLiteral("Roboto"), emojiFamily);
            QFont::insertSubstitution(QStringLiteral("sans-serif"), emojiFamily);
        } else {
            qWarning("NotoColorEmoji.ttf loaded but returned empty font families list");
        }
    }
#else
    qWarning("HAS_EMOJI_FONT is NOT defined at compile time!");
#endif

    SessionController controller;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("sessionController", &controller);
    engine.rootContext()->setContextProperty("emojiFontFamily", emojiFamily);
    const QUrl url(QStringLiteral("qrc:/qt/qml/DecisionHelper/src/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject* obj, const QUrl& objUrl) {
        if (!obj && url == objUrl) QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
