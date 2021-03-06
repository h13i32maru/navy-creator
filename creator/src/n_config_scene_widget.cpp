#include "window/n_text_dialog.h"
#include "n_config_scene_widget.h"
#include "util/n_util.h"
#include "ui_n_config_scene_widget.h"

#include <QMenu>
#include <QMessageBox>
#include <QDebug>

#include <window/n_scene_dialog.h>

NConfigSceneWidget::NConfigSceneWidget(const QString &filePath, QWidget *parent) : NFileWidget(filePath, parent), ui(new Ui::NConfigSceneWidget)
{
    ui->setupUi(this);

    mConfigScene.parseFromFilePath(filePath);
    syncJsonToTree();

    connect(ui->sceneConfigTreeWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    connect(ui->sceneConfigTreeWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(showSceneDialog(QModelIndex)));
}

void NConfigSceneWidget::showSceneDialog(const QModelIndex &/*index*/) {
    QTreeWidgetItem *item = ui->sceneConfigTreeWidget->currentItem();
    if (item == NULL) {
        return;
    }

    QString sceneId = item->text(SCENE_COL_ID);
    NSceneDialog dialog(NSceneDialog::TYPE_UPDATE, mConfigScene, this);
    dialog.setSceneId(sceneId);
    int ret = dialog.exec();
    if (ret == NSceneDialog::Accepted) {
        syncJsonToTree();
        changed();
    }
}

bool NConfigSceneWidget::innerSave() {
    QFile configSceneFile(mFilePath);
    if (!configSceneFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    int ret = configSceneFile.write(this->mConfigScene.stringify());

    return (ret == -1 ? false: true);
}

void NConfigSceneWidget::syncJsonToTree() {
    ui->sceneConfigTreeWidget->clear();

    QList<QTreeWidgetItem *> items;
    for (int i = 0; i < mConfigScene.length(); i++) {
        QString index = QString::number(i);
        NJson scene = mConfigScene.getObject(index);
        QStringList row;
        NUtil::expand(row, ui->sceneConfigTreeWidget->columnCount());
        row[SCENE_COL_ID] = scene.getStr("id");
        row[SCENE_COL_CLASS] = scene.getStr("class");
        row[SCENE_COL_CLASS_FILE] = scene.getStr("classFile");
        row[SCENE_COL_LAYOUT] = scene.getStr("extra.contentLayoutFile");
        row[SCENE_COL_PAGE] = scene.getStr("extra.page");
        QTreeWidgetItem *item = new QTreeWidgetItem(row);
        items.append(item);
    }

    ui->sceneConfigTreeWidget->addTopLevelItems(items);
}

void NConfigSceneWidget::showRawData() {
    NTextDialog dialog(this);
    dialog.setText(mConfigScene.stringify());
    dialog.exec();
}

void NConfigSceneWidget::newScene() {
    NSceneDialog dialog(NSceneDialog::TYPE_CREATE, mConfigScene, this);
    int ret = dialog.exec();
    if (ret == NSceneDialog::Accepted) {
        syncJsonToTree();
        changed();
    }
}

void NConfigSceneWidget::removeScene() {
    QList<QTreeWidgetItem *> selectedItems = ui->sceneConfigTreeWidget->selectedItems();
    if (selectedItems.length() == 0) {
        return;
    }

    QTreeWidgetItem *item = selectedItems[0];

    int ret = QMessageBox::question(NULL, tr(""), tr("do you remove scene?"));

    if (ret == QMessageBox::Yes) {
        int index = mConfigScene.searchValue("id", item->text(SCENE_COL_ID));
        mConfigScene.remove(QString::number(index));
        syncJsonToTree();
        changed();
    }
}

void NConfigSceneWidget::contextMenu(QPoint /*point*/) {
    QMenu menu(this);
    menu.addAction(tr("&New Scene"), this, SLOT(newScene()));
    menu.addAction(tr("&Remove Scene"), this, SLOT(removeScene()));
    menu.addSeparator();
    menu.addAction(tr("&Raw Data"), this, SLOT(showRawData()));
    menu.exec(QCursor::pos());
}

NConfigSceneWidget::~NConfigSceneWidget()
{
    delete ui;
}
