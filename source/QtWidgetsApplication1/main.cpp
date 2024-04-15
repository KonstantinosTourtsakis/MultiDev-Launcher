//#include "QtWidgetsApplication1.h"
#include <QtWidgets/QApplication>

#include <iostream>
#include <vector>
#include <qlabel.h>

//#include <Windows.h>
#include <QDebug>
#include <QFile>
#include <QTextStream>

#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
//#include "ColorWidget.h"
#include <QDirIterator>
#include <QListWidget>

#include <QFileInfo>
#include <QIcon>
#include <QImage>
#include <QFileIconProvider>
#include <QDesktopServices>

#include <QCoreApplication>
#include <QProcess>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QLineEdit>
#include <QTimer>
#include <QGridLayout>
#include <QMenu>
// NEW CODE
#include <QtWidgets>
//#include <QVirtualKeyboard>
#include <QInputMethod>


//#include <QQuickView>
#include <QGuiApplication>
//#include <QQmlEngine>

//#include <QQuickWidget>
//#include <QQmlComponent>

#include <QByteArray>


#include <QMainWindow>

#include <QEvent>
#include <QKeyEvent>



#include <QFileDialog>

//#include <shlobj.h>


#include "keyboard.h"
#include "Controller.h"
#include "CursorMode.h"
#include "GetKeyboardInput.h"



void AllocateConsole()
{
    AllocConsole();
    SetConsoleTitleA("Debug console for Qt6");

    FILE* pCout;
    freopen_s(&pCout, "CONOUT$", "w", stdout);
    freopen_s(&pCout, "CONOUT$", "w", stderr);
    std::cout.clear();
    std::clog.clear();
    std::cerr.clear();
}





// NEW CODE
class Application 
{
public:
    int qt_index;
    QString filename;
    QString window_title;
    QString path;
    std::vector<QString> categories;


};

std::vector<Application> app_list = {};




// NEW CODE
QPalette ui_palette;





// https://stackoverflow.com/questions/12459145/extracting-icon-using-winapi-in-qt-app
QIcon GetFileIcon(const QString& file_path)
{
    SHFILEINFO shfi;
    memset(&shfi, 0, sizeof(SHFILEINFO));

    //if (SHGetFileInfo(reinterpret_cast<const wchar_t*>(file_path.utf16()), 0, &shfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES))
    //if (SHGetFileInfo(reinterpret_cast<const wchar_t*>(file_path.utf16()), 0, &shfi, sizeof(SHFILEINFO), SHGFI_LARGEICON | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES))
    if (SHGetFileInfo(reinterpret_cast<const wchar_t*>(file_path.utf16()), 0, &shfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_USEFILEATTRIBUTES))
    {
        // Scale the icon to a desired size
        //QPixmap pixmap = QPixmap::fromImage(QImage::fromHICON(shfi.hIcon)).scaled(QSize(256, 256), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPixmap pixmap = QPixmap::fromImage(QImage::fromHICON(shfi.hIcon));
        QIcon icon(pixmap);

        // Cleanup the icon resource
        DestroyIcon(shfi.hIcon);

        return icon;
    }

    //return QFileIconProvider().icon(QFileInfo(filePath));
    return QIcon();
}








QString RemoveFileExtension(const QString& fileName) 
{
    int last_dot = fileName.lastIndexOf('.');
    if (last_dot != -1)
    {
        // Return without file extension
        return fileName.left(last_dot); 
    }

    // Return the original
    return fileName;
}












class ApplicationExplorer : public QWidget
{


    QStringList applications_list;
    QStringList current_list;


    // QListWidget to display file names and icons
    QListWidget* list_widget = new QListWidget(this);
    QListWidget* favorite_list = new QListWidget(this);
    QListWidget* directory_list = new QListWidget(this);


public:
    ApplicationExplorer(QApplication& app, QWidget* parent = nullptr) : QWidget(parent), app(app)
    {
        SetupUI();

    }


private:
    QLineEdit* text_input;
    // NEW CODE
    QApplication& app; // Reference to QApplication object
    QComboBox* combo_box;
    QComboBox* cb_profile_switch;
    QCheckBox* cb_file_extension;

    QTimer timer;

    QTabWidget* tabs;
    QPushButton* button_add_tab;
    QPushButton* button_remove_tab;


    void SetupUI()
    {


        tabs = new QTabWidget(this);
        QVBoxLayout* layout_root = new QVBoxLayout(this);

        QWidget* tab_all_apps = new QWidget();
        QWidget* tab_settings = new QWidget();
        QWidget* tab_favorites = new QWidget();



        // NEW CODE
        //QWidget *centralWidget = new QWidget(this);
        //setCentralWidget(centralWidget);



        tabs = new QTabWidget(this);
        layout_root->addWidget(tabs);

        button_add_tab = new QPushButton("Add Tab", this);
        connect(button_add_tab, &QPushButton::clicked, this, &ApplicationExplorer::AddNewTab);
        layout_root->addWidget(button_add_tab);

        button_remove_tab = new QPushButton("Remove Tab", this);
        connect(button_remove_tab, &QPushButton::clicked, this, &ApplicationExplorer::RemoveTab);
        layout_root->addWidget(button_remove_tab);






        // Add tabs to tab widget
        tabs->addTab(tab_all_apps, "Applications");
        tabs->addTab(tab_favorites, "Favorites");
        tabs->addTab(tab_settings, "Settings");
        


        // Box layout to display list_widget items
        //QVBoxLayout* layout_all_apps = new QVBoxLayout(tab_all_apps);
        QGridLayout* layout_all_apps = new QGridLayout(tab_all_apps);
        QGridLayout* layout_favorites = new QGridLayout(tab_favorites);

        // List widget custom arguments
        favorite_list->setViewMode(QListWidget::IconMode);
        favorite_list->setMovement(QListView::Static);
        //list_widget->setDragDropMode(QAbstractItemView::DragDrop);
        favorite_list->setWordWrap(true);
        favorite_list->setWrapping(true);
        favorite_list->setItemAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
        favorite_list->setSpacing(40);
        favorite_list->setIconSize(QSize(64, 64)); // Set the size of the icons
        favorite_list->sortItems();


        layout_root->addWidget(tabs);  // Add the tab widget to the layout
        setLayout(layout_root);
        layout_favorites->addWidget(favorite_list);

        // Create the second tab
        QLabel* label_directories = new QLabel("Directories:");
        QLabel* label_app_theme = new QLabel("Theme");

        QPushButton* button_add_dir = new QPushButton("Add Directory");
        QPushButton* button_remove_dir = new QPushButton("Remove Directory");
        QVBoxLayout* layout_settings = new QVBoxLayout(tab_settings);

        QPushButton* button_save_settings = new QPushButton("Save changes");



        combo_box = new QComboBox(this);
        combo_box->addItem("dark");
        combo_box->addItem("light");
        combo_box->addItem("red");
        combo_box->addItem("green");
        combo_box->addItem("blue");
        combo_box->addItem("yellow");
        combo_box->addItem("orange");
        combo_box->addItem("purple");


        // File extension checkbox
        cb_file_extension = new QCheckBox("Include File Extension", this);
        


        cb_profile_switch = new QComboBox(this);
        cb_profile_switch->addItem("Profile1");
        cb_profile_switch->addItem("Profile2");

        // Load last used profile
        LoadProfile();
        connect(combo_box, &QComboBox::currentTextChanged, this, &ApplicationExplorer::UpdateUIPalette);
        connect(cb_profile_switch, &QComboBox::currentTextChanged, this, &ApplicationExplorer::LoadProfile);



        QHBoxLayout* layout_dir_buttons = new QHBoxLayout();
        layout_dir_buttons->addWidget(button_add_dir);
        layout_dir_buttons->addWidget(button_remove_dir);
        layout_dir_buttons->addWidget(button_save_settings);

        layout_settings->addWidget(label_app_theme);
        layout_settings->addWidget(cb_file_extension);
        layout_settings->addWidget(combo_box);
        layout_settings->addWidget(cb_profile_switch);
        layout_settings->addWidget(label_directories);


        layout_settings->addWidget(directory_list);
        layout_settings->addLayout(layout_dir_buttons);





        // Input field for application searching
        text_input = new QLineEdit(this);
        text_input->setPlaceholderText("Search application");
        text_input->resize(500, 500);
        layout_all_apps->addWidget(text_input);






        current_list = applications_list;



        // List widget custom arguments
        list_widget->setViewMode(QListWidget::IconMode);
        list_widget->setMovement(QListView::Static);
        //list_widget->setDragDropMode(QAbstractItemView::DragDrop);
        list_widget->setWordWrap(true);
        list_widget->setWrapping(true);
        list_widget->setItemAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
        list_widget->setSpacing(40);
        list_widget->setIconSize(QSize(64, 64)); // Set the size of the icons
        list_widget->sortItems();

        //listWidget->setItemDelegate(new IconItemDelegate); // Set the custom item delegate

        layout_all_apps->addWidget(list_widget);
        //layout_all_apps->addWidget(list_widget, 10, 10, Qt::AlignCenter);
        DirectoryListUpdated();
        UpdateListDelayed();




        // CONNECTIONS
        //connect(this, &QListWidget::aboutToQuit, this, &ApplicationExplorer::cleanup);
        connect(list_widget, &QListWidget::itemDoubleClicked, this, &ApplicationExplorer::OnItemDoubleClicked);
        //connect(list_widget, &QListWidget::itemClicked, this, &ApplicationExplorer::OnItemClicked);

        connect(cb_file_extension, &QCheckBox::stateChanged, this, &ApplicationExplorer::UpdateListDelayed);
        // Delay before keyboard input triggers application search
        timer.setSingleShot(true);
        connect(&timer, &QTimer::timeout, this, &ApplicationExplorer::UpdateListDelayed);
        connect(text_input, &QLineEdit::textChanged, this, &ApplicationExplorer::OnTextChanged);
        setContextMenuPolicy(Qt::CustomContextMenu);
        connect(this, &QListWidget::customContextMenuRequested, this, &ApplicationExplorer::ShowContextMenu);

        // Functionality to add directories
        QObject::connect(button_add_dir, &QPushButton::clicked, [this]() {
            QString directory = QFileDialog::getExistingDirectory(nullptr, "Select Directory", QDir::homePath());
            for (int i = 0; i < directory_list->count(); ++i)
            {

                if (directory == directory_list->item(i)->text())
                {
                    return;
                }
            }
            if (!directory.isEmpty())
                directory_list->addItem(directory);
            DirectoryListUpdated();
            UpdateListDelayed();
            });

        // Functionality to remove directories
        QObject::connect(button_remove_dir, &QPushButton::clicked, [this]() {
            qDeleteAll(directory_list->selectedItems());
            DirectoryListUpdated();
            UpdateListDelayed();
            });

        connect(button_save_settings, &QPushButton::clicked, this, &ApplicationExplorer::SaveProfile);

    }



    void SaveProfile()
    {
        QSettings settings("YourCompany", "YourApp");

        settings.beginGroup(cb_profile_switch->itemText(cb_profile_switch->currentIndex()));

        settings.setValue("CTheme", combo_box->currentIndex());

        settings.endGroup();
    }


    void LoadProfile()
    {

        QSettings settings("YourCompany", "YourApp");
        
        settings.beginGroup(cb_profile_switch->itemText(cb_profile_switch->currentIndex()));

        
        int index = settings.value("CTheme", 0).toInt();
        combo_box->setCurrentIndex(index);
        UpdateUIPalette(combo_box->itemText(index));

        settings.endGroup();
    }



    void SaveAppdata()
    {
        QSettings settings("YourCompany", "YourApp");
        
        settings.beginGroup("application");
        
        settings.setValue("last_profile", cb_profile_switch->currentIndex());
        
        settings.endGroup();
    }

    void LoadAppdata()
    {
        QSettings settings("YourCompany", "YourApp");
        
        settings.beginGroup("application");

        // Load the saved index of the combo box
        int index = settings.value("last_profile", 0).toInt();
        cb_profile_switch->setCurrentIndex(index);
        LoadProfile();

        // End reading settings
        settings.endGroup();
    }



    void CleanUp()
    {
        // application cleanup on execution ending
    }






    void ShowContextMenu(const QPoint& pos)
    {
        
        QPoint local_pos = list_widget->mapFromGlobal(pos);

        // Get the index of the item at the local position
        QModelIndex index = list_widget->indexAt(local_pos);

        // Check if the right-click occurred on an item
        if (!index.isValid()) 
        {
            return;
        }


        QListWidgetItem* item = list_widget->currentItem();
        if (!item)
            return;

        QMenu context_menu(tr("Context Menu"), this);

        QAction* action_edit = new QAction(tr("Edit"), this);
        connect(action_edit, &QAction::triggered, this, [this, item]() {
            // Edit action here
            
            });


        QAction* action_delete = new QAction(tr("Remove"), this);
        connect(action_delete, &QAction::triggered, this, [this, item]() 
            {
            
            int item_index = list_widget->row(item);
            app_list.erase(app_list.begin() + item_index);
            delete item;
            });


        QAction* action_add_favorite = new QAction(tr("Add to favorites"), this);
        connect(action_add_favorite, &QAction::triggered, this, [this, item]()
            {
                favorite_list->addItem(item);
                //std::cout << "Added to favorites:" << item->text().toStdString() << std::endl;
            });

        
        QAction* action_add_category = new QAction(tr("Add to Category"), this);
        connect(action_add_category, &QAction::triggered, [this, item]() {
            
            delete item;
            });

        context_menu.addAction(action_edit);
        context_menu.addAction(action_delete);
        context_menu.addAction(action_add_favorite);
        context_menu.addAction(action_add_category);

        
        context_menu.exec(mapToGlobal(pos));
    }



    void ExploreDirectoryFiles(const QString& path)
    {
        QDir dir(path);
        QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

        for (const QFileInfo& file_info : files)
        {
            if (file_info.isDir())
            {
                // Recursive call for subdirectories
                ExploreDirectoryFiles(file_info.filePath());
            }
            else
            {
                QString filename = file_info.fileName();
                if (filename.endsWith(".exe") || filename.endsWith(".lnk"))
                {
                    if (!applications_list.contains(file_info.filePath()))
                    {
                        applications_list.append(file_info.filePath());
                        std::cout << file_info.fileName().toStdString() << std::endl;
                    }
                    
                }
            }
        }
    }

private slots:
    
    
    void AddNewTab()
    {
        QString tab_text = QString("Tab %1").arg(tabs->count() + 1);
        QWidget *new_tab = new QWidget(tabs);
        tabs->addTab(new_tab, tab_text);
    }

    void RemoveTab()
    {

        if (tabs->currentIndex() > 1)
        {
            tabs->removeTab(tabs->currentIndex());
        }


    }

    





    void OnItemDoubleClicked(QListWidgetItem* item) 
    {
        
        


        if (item) 
        {
            QString file_path = item->data(Qt::UserRole).toString();
            //std::cout << file_path.toStdString() << std::endl;
            
            
            
            if (file_path.endsWith(".lnk", Qt::CaseInsensitive))
            {
                // Execute shortcuts using QDesktopServices
                if (!QDesktopServices::openUrl(QUrl::fromLocalFile(file_path)))
                {
                    std::cout << "Failed to open shortcut file:" << file_path.toStdString();
                    exit(1);
                }
                
            }
            else 
            {
                // If it's not a shortcut create a QProcess and start it
                QProcess* process = new QProcess();
                process->start(file_path);

                // Connect signals for asynchronous process handling
                QObject::connect(process, &QProcess::started, []() { });

                QObject::connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                    [](int exitCode, QProcess::ExitStatus exitStatus) {
                        qDebug() << "Process finished with exit code:" << exitCode << "and exit status:" << exitStatus;
                    });
                QObject::connect(process, &QProcess::errorOccurred, [](QProcess::ProcessError error) {
                    qDebug() << "Error occurred:" << error;
                    });
            }

        }
    }


    // Start or restart the timer when text changes - helps avoiding performance issues
    void OnTextChanged()
    {
        timer.start(200);
    }


    
    void UpdateUIPalette(const QString &text)
    {


        if(text == "dark")
        {
            ui_palette.setColor(QPalette::Window, QColor(53, 53, 53));
            ui_palette.setColor(QPalette::WindowText, Qt::white);
            ui_palette.setColor(QPalette::Base, QColor(25, 25, 25));
            ui_palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
            ui_palette.setColor(QPalette::ToolTipBase, Qt::white);
            ui_palette.setColor(QPalette::ToolTipText, Qt::white);
            ui_palette.setColor(QPalette::Text, Qt::white);
            ui_palette.setColor(QPalette::Button, QColor(53, 53, 53));
            ui_palette.setColor(QPalette::ButtonText, Qt::white);
            ui_palette.setColor(QPalette::BrightText, Qt::red);
            ui_palette.setColor(QPalette::Link, QColor(42, 130, 218));
            ui_palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
            ui_palette.setColor(QPalette::HighlightedText, Qt::black);

            
        }
        else if(text == "light")
        {
            ui_palette.setColor(QPalette::Window, Qt::white);
            ui_palette.setColor(QPalette::WindowText, Qt::black);
            ui_palette.setColor(QPalette::Base, Qt::white);
            ui_palette.setColor(QPalette::AlternateBase, Qt::lightGray);
            ui_palette.setColor(QPalette::ToolTipBase, Qt::white);
            ui_palette.setColor(QPalette::ToolTipText, Qt::black);
            ui_palette.setColor(QPalette::Text, Qt::black);
            ui_palette.setColor(QPalette::Button, Qt::white);
            ui_palette.setColor(QPalette::ButtonText, Qt::black);
            ui_palette.setColor(QPalette::BrightText, Qt::red);
            ui_palette.setColor(QPalette::Link, QColor(42, 130, 218));
            ui_palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
            ui_palette.setColor(QPalette::HighlightedText, Qt::white);
            
        }
        else if(text == "blue")
        {
            ui_palette.setColor(QPalette::Window, QColor(20, 20, 40));
            ui_palette.setColor(QPalette::WindowText, Qt::white);
            ui_palette.setColor(QPalette::Base, QColor(30, 30, 60));
            ui_palette.setColor(QPalette::AlternateBase, QColor(20, 20, 40));
            ui_palette.setColor(QPalette::ToolTipBase, Qt::white);
            ui_palette.setColor(QPalette::ToolTipText, Qt::white);
            ui_palette.setColor(QPalette::Text, Qt::white);
            ui_palette.setColor(QPalette::Button, QColor(30, 30, 60));
            ui_palette.setColor(QPalette::ButtonText, Qt::white);
            ui_palette.setColor(QPalette::BrightText, Qt::red);
            ui_palette.setColor(QPalette::Link, QColor(0, 160, 240));
            ui_palette.setColor(QPalette::Highlight, QColor(0, 160, 240));
            ui_palette.setColor(QPalette::HighlightedText, Qt::black);
        }
        else if(text == "green")
        {
            ui_palette.setColor(QPalette::Window, QColor(20, 40, 20));
            ui_palette.setColor(QPalette::WindowText, Qt::white);
            ui_palette.setColor(QPalette::Base, QColor(30, 60, 30));
            ui_palette.setColor(QPalette::AlternateBase, QColor(20, 40, 20));
            ui_palette.setColor(QPalette::ToolTipBase, Qt::white);
            ui_palette.setColor(QPalette::ToolTipText, Qt::white);
            ui_palette.setColor(QPalette::Text, Qt::white);
            ui_palette.setColor(QPalette::Button, QColor(30, 60, 30));
            ui_palette.setColor(QPalette::ButtonText, Qt::white);
            ui_palette.setColor(QPalette::BrightText, Qt::red);
            ui_palette.setColor(QPalette::Link, QColor(0, 160, 0));
            ui_palette.setColor(QPalette::Highlight, QColor(0, 160, 0));
            ui_palette.setColor(QPalette::HighlightedText, Qt::black);
        }
        else if(text == "red")
        {
            ui_palette.setColor(QPalette::Window, QColor(40, 20, 20));
            ui_palette.setColor(QPalette::WindowText, Qt::white);
            ui_palette.setColor(QPalette::Base, QColor(60, 30, 30));
            ui_palette.setColor(QPalette::AlternateBase, QColor(40, 20, 20));
            ui_palette.setColor(QPalette::ToolTipBase, Qt::white);
            ui_palette.setColor(QPalette::ToolTipText, Qt::white);
            ui_palette.setColor(QPalette::Text, Qt::white);
            ui_palette.setColor(QPalette::Button, QColor(60, 30, 30));
            ui_palette.setColor(QPalette::ButtonText, Qt::white);
            ui_palette.setColor(QPalette::BrightText, Qt::red);
            ui_palette.setColor(QPalette::Link, QColor(160, 0, 0));
            ui_palette.setColor(QPalette::Highlight, QColor(160, 0, 0));
            ui_palette.setColor(QPalette::HighlightedText, Qt::black);
        }
        else if (text == "yellow")
        {
            ui_palette.setColor(QPalette::Window, QColor(40, 40, 20));
            ui_palette.setColor(QPalette::WindowText, Qt::white);
            ui_palette.setColor(QPalette::Base, QColor(60, 60, 30));
            ui_palette.setColor(QPalette::AlternateBase, QColor(40, 40, 20));
            ui_palette.setColor(QPalette::ToolTipBase, Qt::white);
            ui_palette.setColor(QPalette::ToolTipText, Qt::white);
            ui_palette.setColor(QPalette::Text, Qt::white);
            ui_palette.setColor(QPalette::Button, QColor(60, 60, 30));
            ui_palette.setColor(QPalette::ButtonText, Qt::white);
            ui_palette.setColor(QPalette::BrightText, Qt::red);
            ui_palette.setColor(QPalette::Link, QColor(160, 160, 0));
            ui_palette.setColor(QPalette::Highlight, QColor(160, 160, 0));
            ui_palette.setColor(QPalette::HighlightedText, Qt::black);
        }
        else if(text == "orange")
        {
            ui_palette.setColor(QPalette::Window, QColor(60, 40, 20));
            ui_palette.setColor(QPalette::WindowText, Qt::white);
            ui_palette.setColor(QPalette::Base, QColor(90, 60, 30));
            ui_palette.setColor(QPalette::AlternateBase, QColor(60, 40, 20));
            ui_palette.setColor(QPalette::ToolTipBase, Qt::white);
            ui_palette.setColor(QPalette::ToolTipText, Qt::white);
            ui_palette.setColor(QPalette::Text, Qt::white);
            ui_palette.setColor(QPalette::Button, QColor(90, 60, 30));
            ui_palette.setColor(QPalette::ButtonText, Qt::white);
            ui_palette.setColor(QPalette::BrightText, Qt::red);
            ui_palette.setColor(QPalette::Link, QColor(255, 128, 0));
            ui_palette.setColor(QPalette::Highlight, QColor(255, 128, 0));
            ui_palette.setColor(QPalette::HighlightedText, Qt::black);

        }
        else if(text == "purple")
        {
            ui_palette.setColor(QPalette::Window, QColor(40, 20, 40));
            ui_palette.setColor(QPalette::WindowText, Qt::white);
            ui_palette.setColor(QPalette::Base, QColor(60, 30, 60));
            ui_palette.setColor(QPalette::AlternateBase, QColor(40, 20, 40));
            ui_palette.setColor(QPalette::ToolTipBase, Qt::white);
            ui_palette.setColor(QPalette::ToolTipText, Qt::white);
            ui_palette.setColor(QPalette::Text, Qt::white);
            ui_palette.setColor(QPalette::Button, QColor(60, 30, 60));
            ui_palette.setColor(QPalette::ButtonText, Qt::white);
            ui_palette.setColor(QPalette::BrightText, Qt::red);
            ui_palette.setColor(QPalette::Link, QColor(128, 0, 128));
            ui_palette.setColor(QPalette::Highlight, QColor(128, 0, 128));
            ui_palette.setColor(QPalette::HighlightedText, Qt::black);
        }


        app.setPalette(ui_palette);
    }


    void DirectoryListUpdated()
    {
        applications_list.clear();

        // Update directories list
        for (int i = 0; i < directory_list->count(); ++i)
        {

            QListWidgetItem* item = directory_list->item(i);

            if (item)
            {
                ExploreDirectoryFiles(item->text());
            }
        }
    }


    void UpdateListDelayed()
    {
        


        // Update the QStringList based on the input on the search bar
        list_widget->clear();
        current_list.clear();
        for (const QString& item : applications_list)
        {
            if (item.toLower().contains(text_input->text().toLower()))
            {
                current_list << item;
            }
        }


        // Add items to the list widget
        for (const QString& path : current_list)
        {
            QFileInfo file_info(path);
            QIcon icon = GetFileIcon(path);
            
            QListWidgetItem* item;

            // Create list widget item using the file name
            if (cb_file_extension->isChecked())
            {
                item = new QListWidgetItem(icon, file_info.fileName());
            }
            else
            {
                item = new QListWidgetItem(icon, RemoveFileExtension(file_info.fileName()));
            }
            
            
            // Store file path as item data
            item->setData(Qt::UserRole, path);
            list_widget->addItem(item);
            
        }

    }



    
};









/*

class VirtualKeyboard : public QMainWindow
{

public:
    VirtualKeyboard()
    {
        setWindowTitle("Virtual Keyboard");
        CreateKeyboardUI();
    }





private:
    QLineEdit* virtual_input;
    QLabel* label_instructions;




    void CreateKeyboardUI()
    {
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);


        QVBoxLayout* main_layout = new QVBoxLayout(centralWidget);

        virtual_input = new QLineEdit(this);
        virtual_input->setReadOnly(true);
        virtual_input->setPlaceholderText("Your input");
        main_layout->addWidget(virtual_input);


        QGridLayout* layout = new QGridLayout();
        main_layout->addLayout(layout);


        const QStringList UpperKeyLayout
        {
            "!", "@", "#", "$", "%", "^", "&", "*", "(", ")",
            "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",
            "A", "S", "D", "F", "G", "H", "J", "K", "L", "_",
            "Z", "X", "C", "V", "B", "N", "M", "<", ">", "?",
        };


        const QStringList LowerKeyLayout
        {
            "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
            "q", "w", "e", "r", "t", "y", "u", "i", "o", "p",
            "a", "s", "d", "f", "g", "h", "j", "k", "l", "-",
            "z", "x", "c", "v", "b", "n", "m", ",", ".", "/",
        };

        QStringList KeyLayout = LowerKeyLayout;



        // counting rows and columns
        int row = 0, column = 0;
        for (const QString& text : KeyLayout)
        {

            QPushButton* button = new QPushButton(text);

            connect(button, &QPushButton::clicked, this, [=]()
                {
                    virtual_input->insert(text);
                });

            layout->addWidget(button, row, column);
            column++;


            if (column == 10)
            {
                column = 0;
                row++;
            }
        }


        // Creating the last row of buttons
        QPushButton* button_back = new QPushButton("Backspace");
        QPushButton* button_clear = new QPushButton("Clear");
        QPushButton* button_left = new QPushButton("<");
        QPushButton* button_right = new QPushButton(">");

        connect(button_back, &QPushButton::clicked, this, [=]()
            {
                virtual_input->backspace();
            });
        column = 3;
        row++;
        layout->addWidget(button_back, row, column);
        connect(button_clear, &QPushButton::clicked, this, [=]()
            {
                virtual_input->clear();
            });
        column++;
        layout->addWidget(button_clear, row, column);

        connect(button_left, &QPushButton::clicked, this, [=]()
            {
                virtual_input->cursorBackward(true, 1);
            });
        column++;
        layout->addWidget(button_left, row, column);

        connect(button_right, &QPushButton::clicked, this, [=]()
            {
                virtual_input->cursorForward(true, 1);
            });
        column++;
        layout->addWidget(button_right, row, column);



        label_instructions = new QLabel("Instructions: Click on the keys to input text", this);
        main_layout->addWidget(label_instructions);
    }

};

*/








class KeyEventListener : public QObject
{

public:
    explicit KeyEventListener(QObject* parent = nullptr) : QObject(parent) {}

protected:
    bool eventFilter(QObject* obj, QEvent* event) override
    {
        if (event->type() == QEvent::KeyPress) 
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            
            if (keyEvent->key() == Qt::Key_Space) 
            {
                std::cout << "Space key pressed\n";
            }
        }

        return QObject::eventFilter(obj, event);
    }
};






bool controller_navigation = true;
bool temp = false; // Used to do some stuff once during the first loop in ControllerNavigation()





void UpdateMainMenu() { return; }








bool init_gamepad = false;




class TaskGamepad : public QObject
{

public:
    
    TaskGamepad(QObject* parent = nullptr) : QObject(parent)
    {
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &TaskGamepad::TaskGamepadNavigation);
        // Perform task constantly
        timer->start(0);
    }


    void SetQKeyboard(VirtualKeyboard* app)
    {
        QKeyboard = app;
    }
    



    
    void ControllerNavigation()
    {

        //Toggle controller navigation (the user wouldn't want the controller navigation to be working while gaming)
        if (user->IsButtonDown(GAMEPAD_BACK) && user->IsButtonJustDown(GAMEPAD_RIGHT_THUMB))
        {
            controller_navigation = !controller_navigation;
            UpdateMainMenu();
            Beep(523, 500);
        }



        //Open the virtual keboard to type an input and send it
        if (user->IsButtonDown(GAMEPAD_BACK) && user->IsButtonJustDown(GAMEPAD_RB))
        {
            getting_input = true;
            
            //QKeyboard->show();
            QKeyboard->showMaximized();
            UpdateMainMenu();
        }


        if (getting_input)
        {
            QKeyboard->SendKeyboardInput();
        }




        if (controller_navigation)
        {
            Reset_Key_Press_Counter(); // Used in Smooth scrolling/navigating with the arrows/D-Pad --- must be called once every loop
            CursorMode();
        }

    }


public slots:
    void TaskGamepadNavigation()
    {
        
        // Perform gamepad initialization (once)
        if (!init_gamepad)
        {
            user = new XBOXController(1); // Initizialize the controller object
            init_gamepad = true;
        }


        if (user->IsConnected())
        {
            // Getting trigger values
            user->left_trigger = user->GetState().Gamepad.bLeftTrigger / 255.0f;
            user->right_trigger = user->GetState().Gamepad.bRightTrigger / 255.0f;
            ControllerNavigation();
            

            /*if (user->IsButtonJustDown(GAMEPAD_B)) //B
            {
                user->Vibrate(65535, 0);
                cout << "B is just down" << endl;
            }*/
        }
        /*else
        {

            menu_controller_color = 12;
            UpdateMainMenu();

            if (user->IsConnected())
            {
                menu_controller_color = 10; // reseting the color in case the controller is connected afterwards
                UpdateMainMenu(); //Updating the menu again cause it only gets updated when needed
            }

        } */
    }

private:
    QTimer* timer;
    VirtualKeyboard* QKeyboard;
};





class MainWindow : public QMainWindow
{

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    QListWidget* listWidget;

    void setupContextMenu();

private slots:
    void showContextMenu(const QPoint& pos);
    void printFilename();
};





MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    listWidget = new QListWidget(this);
    setCentralWidget(listWidget);

    // Load files from a directory
    QDir directory("C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs\\Shortcuts");
    QStringList fileList = directory.entryList(QDir::Files);
    listWidget->addItems(fileList);

    // Set icon mode
    listWidget->setViewMode(QListWidget::IconMode);

    // Set context menu policy
    listWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    // Connect context menu signal
    connect(listWidget, &QListWidget::customContextMenuRequested, this, &MainWindow::showContextMenu);

    setupContextMenu();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupContextMenu()
{
    QAction* printAction = new QAction("Print Filename", this);
    connect(printAction, &QAction::triggered, this, &MainWindow::printFilename);

    listWidget->addAction(printAction);
}

void MainWindow::showContextMenu(const QPoint& pos)
{
    QPoint globalPos = listWidget->mapToGlobal(pos);

    QMenu contextMenu;
    contextMenu.addAction("Print Filename");

    contextMenu.exec(globalPos);
}

void MainWindow::printFilename()
{
    QListWidgetItem* item = listWidget->currentItem();
    if (item) {
        QString filename = item->text();
        std::cout << "Filename: " << filename.toStdString() << std::endl;
    }
}



/*

int main(int argc, char* argv[])
{
    AllocateConsole();
    QApplication a(argc, argv);
    MainWindow w;
    w.showMaximized();
    return a.exec();
}





*/






int main(int argc, char* argv[]) 
{
    AllocateConsole();
    //qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));
    
    QApplication app(argc, argv);
    

    ApplicationExplorer explorer(app);

    
    VirtualKeyboard QKeyboard;
    QKeyboard.setWindowFlags(Qt::WindowCloseButtonHint | Qt::FramelessWindowHint);

    TaskGamepad gamepad;
    // Pass the window object to the gamepad task to handle virtual keyboard input
    gamepad.SetQKeyboard(&QKeyboard);



    //explorer.setWindowFlags(Qt::WindowCloseButtonHint | Qt::FramelessWindowHint);

    
    //explorer.resize(QGuiApplication::primaryScreen()->availableGeometry().size());
    
    //explorer.setWindowState(Qt::WindowMaximized);
    //explorer.showMaximized();
    //QSize window_size = explorer.size();
    
    //explorer.setFixedSize(window_size);

    //explorer.setCentralWidget(tabWidget);
    explorer.setWindowTitle("P2019140 - Konstantinos Tourtsakis");
    
    //explorer.setFixedSize(QGuiApplication::primaryScreen()->availableGeometry().size());
    explorer.showMaximized();


    return app.exec();
}












/*


int main(int argc, char* argv[])
{

    AllocateConsole();


    QApplication app(argc, argv);
    QtWidgetsApplication1 main_window;
    main_window.setWindowTitle("My first Qt Application in Qt6");
    main_window.showMaximized();

    /*
    // Create a ColorWidget
    ColorWidget* colorWidget = new ColorWidget(&main_window);

    // Create a button to change color
    QPushButton* button = new QPushButton("Change Color", &main_window);
    QObject::connect(button, &QPushButton::clicked, [&]() {
        // Set a random color
        int r = rand() % 256;
        int g = rand() % 256;
        int b = rand() % 256;
        colorWidget->setColor(r, g, b);
        });

    button->move(100, 10);

    // Set layout
    QVBoxLayout* layout = new QVBoxLayout(&main_window);
    layout->addWidget(colorWidget);
    layout->addWidget(button);
    //main_window.setLayout(layout);
    */


    //FunctionIconListWidget();


    // File listing ----------------------------------------------------------------

    /*
    QListWidget listWidget;
    listWidget.setIconSize(QSize(32, 32));
    listWidget.setViewMode(QListWidget::IconMode);
    // Recursively iterate over the directory
    for (const auto& dir : directories)
    {
        QDirIterator it(dir, QStringList() << "*.exe" << "*.lnk", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext())
        {
            it.next();
            QString file_path = it.filePath();
            QString filename = it.fileName();
            

            QIcon icon = GetFileIcon(file_path);
            QListWidgetItem* item = new QListWidgetItem(icon, filename);

            // Load icon from file
            //QIcon icon("C:\\Users\\kosta\\Documents\\Icons\\CJ.png");
            
            //std::cout << filename.toStdString() << std::endl << file_path.toStdString() << std::endl;
            
            //item->setIcon(icon);
            listWidget.addItem(file_path);
        }
    }

    //std::vector<Application> apps = {};
    for (int i = 0; i < listWidget.count(); ++i)
    {
        QListWidgetItem* item = listWidget.item(i);
        qDebug() << "Item at index" << i << ":" << item->text();
        std::cout << item->text().toStdString() << std::endl;
        //apps.push_back({ item->text().toStdString(), item->text().toStdString(), item->text().toStdString(), "fav" })
    }


    main_window.setCentralWidget(&listWidget);
    // Set selected list item
    listWidget.setCurrentRow(0);
    listWidget.setSelectionMode(QAbstractItemView::SingleSelection);


    //main_window.setWindowTitle("A list of my files");
    listWidget.show();


    // File listing ----------------------------------------------------------------

    */



/*

    main_window.show();
    return app.exec();
}
*/