#include "n_project.h"
#include "util/n_util.h"
#include "util/n_json.h"

#include <QFileInfo>
#include <QDebug>

#include <window/n_build_error_dialog.h>

NProject* NProject::mInstance = NULL;

NProject* NProject::instance() {
    if (mInstance == NULL) {
        mInstance = new NProject();
    }

    return mInstance;
}

NProject::NProject()
{
}

void NProject::setProject(const QString &projectDirPath) {
    mProject.setPath(projectDirPath);
}

bool NProject::validate() {
    QStringList result;

    {
        QStringList codeList = codes();
        QStringList layoutList = layouts();
        QStringList pageList = pages();

        NJson sceneConfig;
        sceneConfig.parseFromFilePath(mProject.absoluteFilePath("config/scene.json"));
        for (int i = 0; i < sceneConfig.length(); i++) {
            QString index = QString::number(i);
            QString sceneId = sceneConfig.getStr(index + ".id");

            QString classFilePath = sceneConfig.getStr(index + ".classFile");
            if (codeList.indexOf(classFilePath) == -1) {
                result << QString("error: code[%1] used by scene[%2] is not exists.").arg(classFilePath).arg(sceneId);
            }

            QString layoutPath = sceneConfig.getStr(index + ".extra.contentLayoutFile");
            if (layoutList.indexOf(layoutPath) == -1) {
                result << QString("error: layout[%1] used by scene[%2] is not exists.").arg(layoutPath).arg(sceneId);
            }

            QString page = sceneConfig.getStr(index + ".extra.page");
            if (pageList.indexOf(page) == -1) {
                result << QString("error: page[%1] used by scene[%2] is not exists.").arg(page).arg(sceneId);
            }

        }

        NJson pageConfig;
        pageConfig.parseFromFilePath(mProject.absoluteFilePath("config/page.json"));
        for (int i = 0; i < sceneConfig.length(); i++) {
            QString index = QString::number(i);
            QString pageId = pageConfig.getStr(index + ".id");

            QString classFilePath = pageConfig.getStr(index + ".classFile");
            if (codeList.indexOf(classFilePath) == -1) {
                result << QString("error: code[%1] used by page[%2] is not exists.").arg(classFilePath).arg(pageId);
            }

            QString layoutPath = pageConfig.getStr(index + ".extra.contentLayoutFile");
            if (layoutList.indexOf(layoutPath) == -1) {
                result << QString("error: layout[%1] used by page[%2] is not exists.").arg(layoutPath).arg(pageId);
            }
        }
    }

    if (result.length() != 0) {
        NBuildErrorDialog dialog;
        dialog.setError(result);
        dialog.exec();
        return false;
    }

    return true;
}

QStringList NProject::scenes() {
    QString path = mProject.absoluteFilePath("config/scene.json");
    NJson sceneConfig;
    sceneConfig.parseFromFilePath(path);

    QStringList scenes;
    for (int i = 0; i < sceneConfig.length(); i++) {
        QString index = QString::number(i);
        QString scene = sceneConfig.getObject(index).getStr("id");
        scenes.append(scene);
    }

    return scenes;
}

QStringList NProject::pages() {
    QString path = mProject.absoluteFilePath("config/page.json");
    NJson pageConfig;
    pageConfig.parseFromFilePath(path);

    QStringList pages;
    for (int i = 0; i < pageConfig.length(); i++) {
        QString index = QString::number(i);
        QString page = pageConfig.getObject(index).getStr("id");
        pages.append(page);
    }

    return pages;
}

QStringList NProject::images() {
    QStringList images =  NUtil::recursiveEntryList(mProject.absoluteFilePath("image"), "image/");
    return images;
}

QStringList NProject::codes() {
    QStringList codes =  NUtil::recursiveEntryList(mProject.absoluteFilePath("code"), "code/");
    return codes;
}

QStringList NProject::layouts() {
    QStringList layouts =  NUtil::recursiveEntryList(mProject.absoluteFilePath("layout"), "layout/");
    return layouts;
}

QStringList NProject::files() {
    QStringList list;
    list.append(images());
    list.append(codes());
    list.append(layouts());
    return list;
}

bool NProject::existsFile(const QString &relativePath) {
    QString path = mProject.absoluteFilePath(relativePath);
    return QFileInfo(path).exists();
}

bool NProject::existsPage(const QString &page) {
    QStringList pages = this->pages();
    int index = pages.indexOf(page);
    return index == -1 ? false : true;
}

QString NProject::absoluteFilePath(const QString &relativePath) {
    return mProject.absoluteFilePath(relativePath);
}
