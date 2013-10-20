#ifndef N_LAYOUT_WIDGET_H
#define N_LAYOUT_WIDGET_H

#include "n_file_widget.h"
#include "native_bridge.h"
#include "util/n_json.h"

#include <QTreeWidgetItem>

namespace Ui {
class NLayoutWidget;
}

class NLayoutWidget : public NFileWidget
{
    Q_OBJECT

public:
    enum ViewsCol {ViewsColId, ViewsColClass};
    enum ViewClassCol {ViewClassColName, ViewClassColClass};
    static const QString HtmlFilePath;

    explicit NLayoutWidget(const QString &filePath, QWidget *parent = 0);
    virtual void refreshForActive();
    ~NLayoutWidget();

protected:
    virtual bool innerSave();

private:
    Ui::NLayoutWidget *ui;
    NativeBridge *mNative;

    QString contentLayoutJsonText() const;

private slots:
    void contextMenuForViewsTree(const QPoint &point);
    void contextMenuForWebView(const QPoint &point);
    void injectNativeBridge();
    void setViewsFromJS(const NJson &views);
    void updateViewsToJS(QTreeWidgetItem *droppedItem);
    void selectViewToJS();
    void addViewToJS(QTreeWidgetItem *item, int index);
    void deleteViewToJS();
    void setScreenToJS();
    void setCurrentViewFromJS(const NJson &json);
    void setScreenEnable(bool enable);

    // for webview
    void reload();
    void showRawData();
    void showInspector();
};

#endif // N_LAYOUT_WIDGET_H