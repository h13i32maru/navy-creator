#ifndef N_CONFIG_APP_WIDGET_H
#define N_CONFIG_APP_WIDGET_H

#include "n_file_widget.h"
#include "util/n_json.h"

#include <QWidget>
#include <QDir>

namespace Ui {
class NConfigAppWidget;
}

class NConfigAppWidget : public NFileWidget
{
    Q_OBJECT

public:
    explicit NConfigAppWidget(const QString &filePath, QWidget *parent = 0);
    ~NConfigAppWidget();
    virtual void refreshForActive();

protected:
    virtual bool innerSave();

private:
    Ui::NConfigAppWidget *ui;
    NJson mConfigApp;

    void syncWidgetToJson();
    void syncJsonToWidget();

private slots:
    void contextMenu(QPoint point);
    void showRawData();
};

#endif // N_CONFIG_APP_WIDGET_H
