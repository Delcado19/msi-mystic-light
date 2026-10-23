#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setApplicationName("MSI Mystic Light Standalone");
    // The scaffold deliberately has no device transport or startup writes.
    return 0;
}
