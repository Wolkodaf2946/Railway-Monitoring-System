#include <QApplication>
#include <QColor>
#include <QPalette>

#include "ui/MainWindow.h"

namespace {

void applyLightTheme(QApplication &app)
{
    app.setStyle(QStringLiteral("Fusion"));

    QPalette palette;
    palette.setColor(QPalette::Window, QColor(240, 240, 240));
    palette.setColor(QPalette::WindowText, Qt::black);
    palette.setColor(QPalette::Base, Qt::white);
    palette.setColor(QPalette::AlternateBase, QColor(245, 245, 245));
    palette.setColor(QPalette::Text, Qt::black);
    palette.setColor(QPalette::Button, QColor(240, 240, 240));
    palette.setColor(QPalette::ButtonText, Qt::black);
    palette.setColor(QPalette::Link, QColor(0, 120, 215));
    palette.setColor(QPalette::Highlight, QColor(0, 120, 215));
    palette.setColor(QPalette::HighlightedText, Qt::white);

    app.setPalette(palette);
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    applyLightTheme(app);

    MainWindow window;
    window.show();

    return app.exec();
}
