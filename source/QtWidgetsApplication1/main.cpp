//#include "QtWidgetsApplication1.h"
#include <QtWidgets/QApplication>

#include <iostream>
#include <vector>
#include <qlabel.h>
#include <QDebug>
#include <QFile>
#include <QTextStream>

#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
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
#include <QtWidgets>
#include <QInputMethod>


#include <QGuiApplication>

#include <QByteArray>


#include <QMainWindow>

#include <QEvent>
#include <QCompleter>
#include <QFileDialog>


#include "keyboard.h"
#include "Controller.h"
#include "CursorMode.h"
#include "GetKeyboardInput.h"








void AllocateConsole(const LPCSTR console_title)
{
    AllocConsole();
    SetConsoleTitleA(console_title);

    FILE* pCout;
    freopen_s(&pCout, "CONOUT$", "w", stdout);
    freopen_s(&pCout, "CONOUT$", "w", stderr);
    std::cout.clear();
    std::clog.clear();
    std::cerr.clear();
}






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

    
    return QIcon();
}








QString RemoveFileExtension(const QString& file_name) 
{
    int last_dot = file_name.lastIndexOf('.');

    if (last_dot != -1)
    {
        // Return without file extension
        return file_name.left(last_dot);
    }

    // Return the original
    return file_name;
}







int CalculateWidthPercentage(double percentage) 
{
    QScreen* primary_screen = QGuiApplication::primaryScreen();
    QRect screen_geometry = primary_screen->geometry();
    
    return static_cast<int>(percentage * screen_geometry.width() / 100.0);
}





// Some global controller variables
bool controller_navigation = true;
// Used to do some stuff once during the first loop in ControllerNavigation()
bool temp = false;
bool init_gamepad = false;
// Global palette for the UI
QPalette ui_palette;



class ApplicationExplorer : public QWidget
{


    
public:
    VirtualKeyboard* QKeyboard;
    ApplicationExplorer(QApplication& app, QWidget* parent = nullptr) : QWidget(parent), app(app)
    {
        
        
        // Controller task loop - Perform task constantly
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &ApplicationExplorer::TaskGamepadNavigation);
        timer->start(0);
        
        
        //SetupScreen();
        //SetupUI();

    }

    void DoTheUI()
    {
        
        this->SetupUI();
    }

private:
    QStringList applications_list;
    QStringList favorites_list;
    QListWidget* list_widget = new QListWidget(this);
    QListWidget* list_favorites = new QListWidget(this);
    QListWidget* search_list = new QListWidget(this);
    QListWidget* directory_list = new QListWidget(this);
    QVBoxLayout* search_layout;
    QLineEdit* search_bar;
    QTimer* timer;


    QLineEdit* line_all_search;
    QLineEdit* line_fav_search;

    // Reference to QApplication object
    QApplication& app; 
    QComboBox* cb_theme;
    QComboBox* cb_profile_switch;
    QCheckBox* chb_file_extension;

    QTimer timer_search;
    QTimer* communication_task_timer;

    QGridLayout* layout_all_apps;

    QTabWidget* tabs;
    QPushButton* button_add_tab;
    QPushButton* button_remove_tab;
    QWidget search_window;
    


    // QMap to store usage frequency of each application
    QMap<QString, int> app_usage_freq; 
    QStringList popular_apps;
    QCompleter* completer;



    void UpdatePopularAppsList()
    {
        // Sort application file paths based on usage frequency
        QList<QString> sorted_apps = app_usage_freq.keys();
        std::sort(sorted_apps.begin(), sorted_apps.end(), [&](const QString& a, const QString& b) 
            {
                return app_usage_freq[a] > app_usage_freq[b];
            });

        // Get top 10 most used applications
        popular_apps = sorted_apps.mid(0, 10);
    }

    
    
    void SetupUI()
    {
        
        QVBoxLayout* layout_root = new QVBoxLayout(this);
        setLayout(layout_root);
        

        tabs = new QTabWidget(this);
        // Tabs
        QWidget* tab_all_apps = new QWidget();
        QWidget* tab_settings = new QWidget();
        QWidget* tab_favorites = new QWidget();


        // Layouts
        //QVBoxLayout* layout_all_apps = new QVBoxLayout(tab_all_apps);
        layout_all_apps = new QGridLayout(tab_all_apps);
        QGridLayout* layout_favorites = new QGridLayout(tab_favorites);
        QVBoxLayout* layout_settings = new QVBoxLayout(tab_settings);


        
        





        // Add tabs to tab widget
        tabs->addTab(tab_all_apps, "Applications");
        tabs->addTab(tab_favorites, "Favorites");
        tabs->addTab(tab_settings, "Settings");
        
        layout_root->addWidget(tabs);

        


        // List widget custom arguments
        list_favorites->setViewMode(QListWidget::IconMode);
        list_favorites->setMovement(QListView::Static);
        list_favorites->setWordWrap(true);
        list_favorites->setWrapping(true);
        list_favorites->setItemAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
        list_favorites->setSpacing(40);
        list_favorites->setIconSize(QSize(64, 64));
        list_favorites->sortItems();


        
        
        

        // Create the second tab
        QLabel* label_directories = new QLabel("Directories:");
        QLabel* label_app_theme = new QLabel("Theme");

        QPushButton* button_add_dir = new QPushButton("Add Directory");
        QPushButton* button_remove_dir = new QPushButton("Remove Directory");
        QPushButton* button_save_settings = new QPushButton("Save changes");



        cb_theme = new QComboBox(this);
        cb_theme->addItem("dark");
        cb_theme->addItem("light");
        cb_theme->addItem("red");
        cb_theme->addItem("green");
        cb_theme->addItem("blue");
        cb_theme->addItem("yellow");
        cb_theme->addItem("orange");
        cb_theme->addItem("purple");


        // File extension checkbox
        chb_file_extension = new QCheckBox("Include File Extension", this);
        
        QFont font("Arial", 18);

        cb_profile_switch = new QComboBox(this);
        cb_profile_switch->addItem("Profile1");
        cb_profile_switch->addItem("Profile2");

        QLineEdit* line_add_profile = new QLineEdit(this);
        line_add_profile->setPlaceholderText("Add profile");
        line_add_profile->setFixedHeight(50);
        line_add_profile->setFont(font);



        QHBoxLayout* layout_dir_buttons = new QHBoxLayout();
        layout_dir_buttons->addWidget(button_add_dir);
        layout_dir_buttons->addWidget(button_remove_dir);
        

        layout_settings->addWidget(label_app_theme);
        layout_settings->addWidget(chb_file_extension);
        layout_settings->addWidget(cb_theme);
        layout_settings->addWidget(cb_profile_switch);
        layout_settings->addWidget(line_add_profile);
        
        layout_settings->addWidget(label_directories);
        layout_settings->addWidget(directory_list);
        layout_settings->addLayout(layout_dir_buttons);
        layout_settings->addWidget(button_save_settings);



        

        // Input field for application searching
        line_all_search = new QLineEdit(this);
        line_all_search->setPlaceholderText("Search application");
        line_all_search->setFixedHeight(50);
        line_all_search->setFont(font);

        line_fav_search = new QLineEdit(this);
        line_fav_search->setPlaceholderText("Search application");
        line_fav_search->setFixedHeight(50);
        line_fav_search->setFont(font);


        

        // List widget custom arguments
        list_widget->setViewMode(QListWidget::IconMode);
        list_widget->setMovement(QListView::Static);
        list_widget->setWordWrap(true);
        list_widget->setWrapping(true);
        list_widget->setItemAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
        list_widget->setSpacing(40);
        list_widget->setIconSize(QSize(64, 64));
        //list_widget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        //list_widget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        
        list_widget->sortItems();

        
        layout_all_apps->addWidget(line_all_search);
        layout_all_apps->addWidget(list_widget);
        layout_favorites->addWidget(line_fav_search);
        layout_favorites->addWidget(list_favorites);
        
        

        // Load last used profile
        LoadProfile();
        connect(cb_theme, &QComboBox::currentTextChanged, this, &ApplicationExplorer::UpdateUIPalette);
        connect(cb_profile_switch, &QComboBox::currentTextChanged, this, &ApplicationExplorer::LoadProfile);


        //for (const QString& app : popular_apps) { std::cout << app.toStdString() << std::endl; }
        UpdatePopularAppsList();
        completer = new QCompleter(popular_apps);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        line_all_search->setCompleter(completer);

        



        // CONNECTIONS
        QObject::connect(&app, &QCoreApplication::aboutToQuit, this, &ApplicationExplorer::CleanUp);
        connect(list_widget, &QListWidget::itemActivated, this, &ApplicationExplorer::ExecuteApplication);
        connect(list_favorites, &QListWidget::itemActivated, this, &ApplicationExplorer::ExecuteApplication);
        
        //QObject::connect(cb_profile_switch, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ApplicationExplorer::SaveProfile);

        //connect(chb_file_extension, &QCheckBox::stateChanged, this, [this] {ApplicationExplorer::UpdateListWidget(applications_list, list_widget); });
        
        //// Delay before keyboard input triggers application search
        timer_search.setSingleShot(true);
        //connect(&timer_search, &QTimer::timeout, this, [this] {ApplicationExplorer::UpdateListWidget(applications_list, list_widget); });
        connect(line_all_search, &QLineEdit::textChanged, this, [this] {ApplicationExplorer::SearchApplication(line_all_search, list_widget, applications_list); });
        connect(line_fav_search, &QLineEdit::textChanged, this, [this] {ApplicationExplorer::SearchApplication(line_fav_search, list_favorites, favorites_list); });
        connect(line_add_profile, &QLineEdit::returnPressed, this, [this, line_add_profile] {
            if (line_add_profile->text() != "")
            {
                cb_profile_switch->addItem(line_add_profile->text());
                cb_profile_switch->setCurrentIndex(cb_profile_switch->count() - 1);
                
            }
            });

        setContextMenuPolicy(Qt::CustomContextMenu);
        connect(this, &QListWidget::customContextMenuRequested, this, [this] {
            ApplicationExplorer::ShowContextMenu(QCursor::pos(), list_widget);
            });
        connect(this, &QListWidget::customContextMenuRequested, this, [this] {
            ApplicationExplorer::ShowContextMenu(QCursor::pos(), list_favorites);
            });

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
            UpdateListWidget(applications_list, list_widget);
            });

        // Functionality to remove directories
        QObject::connect(button_remove_dir, &QPushButton::clicked, [this]() {
            qDeleteAll(directory_list->selectedItems());
            DirectoryListUpdated();
            UpdateListWidget(applications_list, list_widget);
            });

        connect(button_save_settings, &QPushButton::clicked, this, &ApplicationExplorer::SaveProfile);


        
        // ULauncher-like window for application searching through the keyboard
        search_window.setWindowTitle("Qt6ULauncher");
        search_layout = new QVBoxLayout();
        //QVBoxLayout* sea_layout = new QVBoxLayout();
        search_window.setLayout(search_layout);
        search_window.resize(300, 40);
        search_window.setWindowFlags(Qt::WindowCloseButtonHint | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        

        // Create UI elements
        search_bar = new QLineEdit(this);
        search_bar->setFixedHeight(50);
        search_bar->setFont(font);
        search_list->setViewMode(QListView::ListMode);
        search_list->hide();
        search_list->setFixedHeight(50);
        search_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        search_list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        
        search_layout->addWidget(search_bar);
        search_layout->addWidget(search_list);
        
        connect(search_bar, &QLineEdit::textChanged, this, &ApplicationExplorer::SearchResults);
        connect(search_list, &QListWidget::itemActivated, this, &ApplicationExplorer::ExecuteApplication);
        

        communication_task_timer = new QTimer(this);
        connect(communication_task_timer, &QTimer::timeout, this, &ApplicationExplorer::TaskHandleDevicesUICommunication);
        // Perform task constantly
        communication_task_timer->start(0);

        DirectoryListUpdated();
        UpdateListWidget(applications_list, list_widget);

        line_all_search->setText("grand");
        SearchApplication(line_all_search, list_widget, applications_list);
        line_all_search->setText("su");
        SearchApplication(line_all_search, list_widget, applications_list);
        
        for (int row = 0; row < layout_all_apps->rowCount(); ++row) 
        {
            for (int column = 0; column < layout_all_apps->columnCount(); ++column) 
            {
                QLayoutItem* layoutItem = layout_all_apps->itemAtPosition(row, column);
                if (layoutItem) 
                {
                    std::cout << "Item at: " << row << column << std::endl;
                    
                    QWidget* widget = layoutItem->widget();
                    
                    if (widget) 
                    {
                        // Here you can perform operations with the widget
                        // For example, you can check if it's a QListWidget item
                    }
                }
            }
        }

    }

    
    void TaskHandleDevicesUICommunication()
    {


        // Handling keyboard application launcher window hardware - software communication
        if (IsKeyDown(VK_CONTROL) && IsKeyJustDown(VK_SPACE))
        {
            if (search_window.isVisible())
            {
                search_window.hide();
            }
            else
            {
                search_bar->clear();
                search_window.show();
                search_window.activateWindow();
                search_window.raise();
                search_window.setFocus();
                search_bar->setEnabled(true);
                search_bar->setFocus();
            }

            return;
        }



        if (!search_list->hasFocus() && (IsKeyJustDown(VK_DOWN) || IsKeyJustDown(VK_UP)))
        {
            search_list->setFocus();
            search_list->setCurrentRow(0);
        }


        // Checks to return to keyboard input
        for (int i = 'A'; i <= 'Z'; ++i)
        {
            if (IsKeyJustDown(i))
            {
                search_bar->setFocus();
            }
        }

        if (IsKeyJustDown(VK_SPACE) || IsKeyJustDown(VK_BACK) || IsKeyJustDown(VK_LEFT) || IsKeyJustDown(VK_RIGHT))
        {
            search_bar->setFocus();
        }



        // Handling controller navigation - UI communication


        if (user->IsButtonJustDown(GAMEPAD_DPAD_UP) && list_widget->currentRow() == 0)
        {
            line_all_search->setFocus();
        }

        if (user->IsButtonJustDown(GAMEPAD_DPAD_DOWN) && line_all_search->hasFocus())
        {
            list_widget->setFocus();
        }

        if (user->IsButtonJustDown(GAMEPAD_LB))
        {
            tabs->setCurrentIndex(0);
        }

        if (user->IsButtonJustDown(GAMEPAD_RB))
        {
            tabs->setCurrentIndex(1);
        }




    }


    
    // Searching for the ULauncher-like window
    void SearchResults(const QString& text)
    {
        search_list->clear();

        if (text.isEmpty())
        {
            search_list->hide();
            //search_window.resize(300, 40);
            search_layout->update();
            search_list->update();
            std::cout << "Case 1" << std::endl;
            return;
        }

        int max_results = 10;
        int count = 0;

        for (int i = 0; i < list_widget->count(); ++i)
        {
            QListWidgetItem* item = list_widget->item(i);

            if (item->text().contains(text, Qt::CaseInsensitive))
            {

                QString path = item->data(Qt::UserRole).toString();
                QFileInfo file_info(path);
                QIcon icon = GetFileIcon(path);

                QListWidgetItem* item2;

                // Create list widget item using the file name or file extension
                if (chb_file_extension->isChecked())
                {
                    item2 = new QListWidgetItem(icon, file_info.fileName());
                }
                else
                {
                    item2 = new QListWidgetItem(icon, RemoveFileExtension(file_info.fileName()));
                }


                // Store file path as item data
                item2->setData(Qt::UserRole, path);
                search_list->addItem(item2);
                
                ++count;
                if (count >= max_results)
                {
                    break;
                }
            }
        }


        if (search_list->count() == 0)
        {
            std::cout << "Case 2" << std::endl;
            search_list->hide();
            //search_window.resize(300, 40);
        }
        else
        {
            std::cout << "Case 3" << std::endl;
            search_list->show();
            //search_window.resize(300, 400);
        }

        search_layout->update();
        search_list->update();
    }


    
    void SaveProfile()
    {
        QSettings settings("MyQt6Application1");

        settings.beginGroup(cb_profile_switch->itemText(cb_profile_switch->currentIndex()));

        settings.setValue("Theme", cb_theme->currentIndex());
        //settings.setValue("popular_apps", popular_apps);
        
        // Convert the QMap to a QVariantMap
        QVariantMap map;
        QMapIterator<QString, int> iter(app_usage_freq);
        while (iter.hasNext()) 
        {
            iter.next();
            map.insert(iter.key(), iter.value());
        }
        settings.setValue("app_frequency", map);

        QStringList directories;

        // Iterate through the directory_list and save directory paths to QStringList
        for (int i = 0; i < directory_list->count(); ++i)
        {
            directories << directory_list->item(i)->text();
        }

        // Save directories to settings
        settings.setValue("Directories", directories);


        settings.endGroup();
    }


    void LoadProfile()
    {

        QSettings settings("MyQt6Application1");
        
        settings.beginGroup(cb_profile_switch->itemText(cb_profile_switch->currentIndex()));

        
        //popular_apps = settings.value("popular_apps").toStringList();
        QVariantMap retrievedMap = settings.value("app_frequency").toMap();

        // Convert the QVariantMap back to a QMap<QString, int>
        QMapIterator<QString, QVariant> mapIter(retrievedMap);
        while (mapIter.hasNext()) 
        {
            mapIter.next();
            app_usage_freq.insert(mapIter.key(), mapIter.value().toInt());
        }

        int index = settings.value("Theme", 0).toInt();
        cb_theme->setCurrentIndex(index);
        UpdateUIPalette(cb_theme->itemText(index));


        QStringList directories = settings.value("Directories").toStringList();

        // Clear the listWidget before loading directories
        directory_list->clear();

        // Add directories from settings to the listWidget
        foreach(const QString & dir, directories) 
        {
            directory_list->addItem(dir);
        }

        //DirectoryListUpdated();
        //UpdateListWidget(applications_list, list_widget);
        

        settings.endGroup();
    }



    void SaveAppdata()
    {
        QSettings settings("MyQt6Application1");
        
        settings.beginGroup("application");
        
        settings.setValue("last_profile", cb_profile_switch->currentIndex());
        
        
        settings.endGroup();
    }

    void LoadAppdata()
    {
        QSettings settings("MyQt6Application1");
        
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
        ApplicationExplorer::SaveProfile();
        Beep(200, 200);
    }






    void ShowContextMenu(const QPoint& pos, QListWidget* list)
    {
        
        /*QPoint local_pos = list->mapFromGlobal(pos);

        // Get the index of the item at the local position
        QModelIndex index = list->indexAt(local_pos);
        
        // Check if the right-click occurred on an item
        if (!index.isValid())
        {
            return;
        }
        */

        QListWidgetItem* item = list->currentItem();
        if (!item)
            return;
        std::cout << item->data(Qt::UserRole).toString().toStdString() << std::endl;
        QMenu context_menu(tr("Context Menu"), this);

        QAction* action_edit = new QAction(tr("Add application"), this);
        connect(action_edit, &QAction::triggered, this, [this, item]() {

            
            std::cout << "\n\nPopular apps:\n";
            for (const QString& app : popular_apps)
            {
                std::cout << app.toStdString() << std::endl;
            }

            return;
            QFileDialog dialog;

            dialog.setWindowTitle("Add a missing application");

            dialog.setFileMode(QFileDialog::ExistingFile);
            dialog.setDirectory(QDir::homePath());
            dialog.setNameFilter("Executable Files (*.exe);;Shortcut Files (*.lnk *.url)");

            // Show the dialog and wait for the user's selection
            if (dialog.exec())
            {
                // Get the selected file(s)
                QStringList file_names = dialog.selectedFiles();

                // Iterate over the selected files and do something with them
                foreach(QString name, file_names)
                {
                    std::cout << "Selected file:" << name.toStdString();
                    // Here you can perform operations with the selected file(s)
                }

            }});


        QAction* action_delete = new QAction(tr("Remove"), this);
        connect(action_delete, &QAction::triggered, this, [this, item]()
            {
                delete item;   
            });


        
        bool found = false;

        for (int i = 0; i < list_favorites->count(); ++i)
        {
            
            QListWidgetItem* currentItem = list_favorites->item(i);

            if (currentItem->text() == item->text()) 
            {
                
                found = true;
                break;
            }
        }

        QAction* action_add_favorite;
        if (found)
        {
            action_add_favorite = new QAction(tr("Remove from favorites"), this);

            connect(action_add_favorite, &QAction::triggered, this, [this, item]()
                {
                    if (!item)
                    {
                        std::cout << "Invalid item" << std::endl;
                        return;
                    }

                    
                    int string_index = -1;
                    std::cout << "Target: " << item->data(Qt::UserRole).toString().toStdString() << std::endl;
                    for (int i = 0; i < favorites_list.count(); ++i)
                    {
                        std::cout << "Index: " << i << " Item: " << favorites_list[i].toStdString() << std::endl;
                        if (item->data(Qt::UserRole) == favorites_list[i])
                        {
                            std::cout << "Found at index: " << i << std::endl;
                            string_index = i;
                            break;
                        }
                    }


                    if (string_index != -1)
                    {
                        std::cout << "Removing at: " << string_index << std::endl;
                        favorites_list.removeAt(string_index);
                        
                        UpdateListWidget(favorites_list, list_favorites);
                    }
                    else
                    {
                        std::cout << "Favorites list string index was not found" << std::endl;
                    }
                    return;
                    int index = list_favorites->row(item);
                    QListWidgetItem* item_to_remove = list_favorites->takeItem(index);
                    delete item_to_remove;
                    
                });

        }
        else
        {
            action_add_favorite = new QAction(tr("Add to favorites"), this);

            connect(action_add_favorite, &QAction::triggered, this, [this, item]()
                {
                    if (!item)
                    {
                        std::cout << "Invalid item" << std::endl;
                        return;
                    }

                    QString path = item->data(Qt::UserRole).toString();
                    QFileInfo file_info(path);

                    if (!favorites_list.contains(file_info.filePath()))
                    {
                        
                        favorites_list.append(file_info.filePath());
                        UpdateListWidget(favorites_list, list_favorites);
                    }

                    
                    /*return;
                    // Add item to favorites
                    QString path = item->data(Qt::UserRole).toString();
                    QFileInfo file_info(path);
                    QIcon icon = GetFileIcon(path);

                    QListWidgetItem* item2;

                    // Create list widget item using the file name or file extension
                    if (chb_file_extension->isChecked())
                    {
                        item2 = new QListWidgetItem(icon, file_info.fileName());
                    }
                    else
                    {
                        item2 = new QListWidgetItem(icon, RemoveFileExtension(file_info.fileName()));
                    }


                    // Store file path as item data
                    item2->setData(Qt::UserRole, path);
                    list_favorites->addItem(item2);

                    */

                });
        }
        
        

        
        QAction* action_add_category = new QAction(tr("Add to Category"), this);
        connect(action_add_category, &QAction::triggered, [this, item]() {
            // To do?
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
                if (filename.endsWith(".exe") || filename.endsWith(".lnk") || filename.endsWith(".url"))
                {
                    if (!applications_list.contains(file_info.filePath()))
                    {
                        applications_list.append(file_info.filePath());
                    }
                    
                }
            }
        }
    }





    void UpdateListWidget(QStringList current_list, QListWidget* list_wid)
    {
        list_wid->clear();
        // Add items to the list widget
        for (const QString& path : current_list)
        {
            QFileInfo file_info(path);
            QIcon icon = GetFileIcon(path);

            QListWidgetItem* item;

            // Create list widget item using the file name or file extension
            if (chb_file_extension->isChecked())
            {
                item = new QListWidgetItem(icon, file_info.fileName());
            }
            else
            {
                item = new QListWidgetItem(icon, RemoveFileExtension(file_info.fileName()));
            }


            // Store file path as item data
            item->setData(Qt::UserRole, path);
            list_wid->addItem(item);

        }

        list_wid->update();

        
    }

    

    // Start or restart the timer when text changes - helps avoiding performance issues
    void SearchApplication(QLineEdit* line, QListWidget* list_wid, QStringList app_list)
    {
        
        if (line->text() == "")
        {
            UpdateListWidget(app_list, list_wid);
            timer_search.start(200);
            return;
        }

        // Update the QStringList based on the input on the search bar
        
        QStringList new_list;
        for (const QString& item : app_list)
        {
            if (item.toLower().contains(line->text().toLower()))
            {
                new_list << item;
            }
        }

        UpdateListWidget(new_list, list_wid);
        timer_search.start(200);

    }



    void ExecuteApplication(QListWidgetItem* item) 
    {



        if (item) 
        {
            // Hide the ulauncher-like window if the application was executed through that
            if (search_window.isVisible())
            {
                search_window.hide();
            }


            QString file_path = item->data(Qt::UserRole).toString();
            QString file_name = RemoveFileExtension(item->text());
            
            
            
            if (file_path.endsWith(".lnk", Qt::CaseInsensitive) || file_path.endsWith(".url", Qt::CaseInsensitive))
            {
                // Execute shortcuts using QDesktopServices
                if (!QDesktopServices::openUrl(QUrl::fromLocalFile(file_path)))
                {
                    std::cout << "Failed to open shortcut file:" << file_path.toStdString();
                    return;
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
                        std::cout << "Process finished with exit code:" << exitCode << "and exit status:" << exitStatus;
                    });
                QObject::connect(process, &QProcess::errorOccurred, [](QProcess::ProcessError error) {
                    std::cout << "Error occurred:" << error;
                    });
            }

            app_usage_freq[file_name]++;
        }
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





    QWidget centralWidget;
    void SetupScreen()
    {
        
        

        QVBoxLayout* layout = new QVBoxLayout();
        centralWidget.setLayout(layout);

        QLabel* titleLabel = new QLabel("Welcome to Your Application", this);
        titleLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(titleLabel);

        QLabel* usernameLabel = new QLabel("Please enter your username:", this);
        usernameLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(usernameLabel);


        QLineEdit* usernameInput = new QLineEdit(this);
        layout->addWidget(usernameInput);

        
        //centralWidget.setCentralWidget(centralWidget);
        centralWidget.setWindowTitle("Introduction Screen");

        QPushButton* continueButton = new QPushButton("Continue", this);
        connect(continueButton, &QPushButton::clicked, this, [usernameInput, this] {
            std::cout << "Username: " << usernameInput->text().toStdString() << std::endl;
            centralWidget.close();
            });
        layout->addWidget(continueButton, 0, Qt::AlignCenter);

        
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


    









    void ControllerNavigation()
    {

        //Toggle controller navigation (the user wouldn't want the controller navigation to be working while gaming)
        if (user->IsButtonDown(GAMEPAD_BACK) && user->IsButtonJustDown(GAMEPAD_RIGHT_THUMB))
        {
            controller_navigation = !controller_navigation;
            Beep(523, 500);
        }



        //Open the virtual keboard to type an input and send it
        if (user->IsButtonDown(GAMEPAD_BACK) && user->IsButtonJustDown(GAMEPAD_RB))
        {
            getting_input = true;
            //QKeyboard->show();
            QKeyboard->showMaximized();
        }


        if (getting_input)
        {
            QKeyboard->SendKeyboardInput();
        }




        if (controller_navigation)
        {
            // Used in Smooth scrolling/navigating with the arrows/D-Pad --- must be called once every loop
            ResetKeyPressCounter();
            CursorMode();
        }

    }


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

        }
    }
    
};



















int main(int argc, char* argv[]) 
{
    AllocateConsole("Debug console for Qt6");
    
    QApplication app(argc, argv);
    
    ApplicationExplorer explorer(app);
    explorer.setWindowTitle("P2019140 - Konstantinos Tourtsakis");
    explorer.resize(800, 600);
    explorer.showMaximized();
    
    explorer.DoTheUI();
    
    VirtualKeyboard QKeyboard;
    QKeyboard.setWindowFlags(Qt::WindowCloseButtonHint | Qt::FramelessWindowHint);
    //QKeyboard.showMaximized();

    // Pass the window object to the gamepad task to handle virtual keyboard input
    //explorer.SetQKeyboard(&QKeyboard);
    explorer.QKeyboard = &QKeyboard;
    


    //explorer.setWindowFlags(Qt::WindowCloseButtonHint | Qt::FramelessWindowHint);
    
    
    
    return app.exec();
}








