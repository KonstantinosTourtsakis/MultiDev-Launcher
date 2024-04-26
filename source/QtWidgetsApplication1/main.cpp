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







class ApplicationExplorer : public QWidget
{


    QStringList applications_list;
    QStringList current_list;
    QStringList removed_apps;


    // QListWidget to display file names and icons
    QListWidget* list_widget = new QListWidget(this);
    QListWidget* search_list = new QListWidget(this);
    QListWidget* favorites_list = new QListWidget(this);
    QListWidget* directory_list = new QListWidget(this);
    QVBoxLayout* search_layout;
    QLineEdit* search_bar;

public:
    ApplicationExplorer(QApplication& app, QWidget* parent = nullptr) : QWidget(parent), app(app)
    {
        QScreen* primaryScreen = QGuiApplication::primaryScreen();
        QRect screenGeometry = primaryScreen->geometry();
        setGeometry(screenGeometry);
        showMaximized();

        SetupScreen();
        SetupUI();

    }



private:
    QLineEdit* text_input;

    // Reference to QApplication object
    QApplication& app; 
    QComboBox* cb_theme;
    QComboBox* cb_profile_switch;
    QCheckBox* chb_file_extension;

    QTimer timer_search;
    QTimer* communication_task_timer;

    QTabWidget* tabs;
    QPushButton* button_add_tab;
    QPushButton* button_remove_tab;
    QWidget search_window;
    



    QMap<QString, int> usageFrequency; // Map to store usage frequency of each application
    QStringList popular_apps;
    QCompleter* completer;

    void UpdateMostUsedApplications()
    {
        // Sort application file paths based on usage frequency
        QList<QString> sorted_apps = usageFrequency.keys();
        std::sort(sorted_apps.begin(), sorted_apps.end(), [&](const QString& a, const QString& b) 
            {
                return usageFrequency[a] > usageFrequency[b];
            });

        // Get top 10 most used applications
        popular_apps = sorted_apps.mid(0, 10);
    }

    
    
    void SetupUI()
    {
        
        QVBoxLayout* layout_root = new QVBoxLayout(this);
        tabs = new QTabWidget(this);
        

        QWidget* tab_all_apps = new QWidget();
        QWidget* tab_settings = new QWidget();
        QWidget* tab_favorites = new QWidget();





        
        

        /*
        button_add_tab = new QPushButton("Add Tab", this);
        connect(button_add_tab, &QPushButton::clicked, this, &ApplicationExplorer::AddNewTab);
        layout_root->addWidget(button_add_tab);

        button_remove_tab = new QPushButton("Remove Tab", this);
        connect(button_remove_tab, &QPushButton::clicked, this, &ApplicationExplorer::RemoveTab);
        layout_root->addWidget(button_remove_tab);
        */





        // Add tabs to tab widget
        tabs->addTab(tab_all_apps, "Applications");
        tabs->addTab(tab_favorites, "Favorites");
        tabs->addTab(tab_settings, "Settings");
        
        layout_root->addWidget(tabs);

        // Box layout to display list_widget items
        //QVBoxLayout* layout_all_apps = new QVBoxLayout(tab_all_apps);
        QGridLayout* layout_all_apps = new QGridLayout(tab_all_apps);
        QGridLayout* layout_favorites = new QGridLayout(tab_favorites);
        QVBoxLayout* layout_settings = new QVBoxLayout(tab_settings);


        // List widget custom arguments
        favorites_list->setViewMode(QListWidget::IconMode);
        favorites_list->setMovement(QListView::Static);
        favorites_list->setWordWrap(true);
        favorites_list->setWrapping(true);
        favorites_list->setItemAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
        favorites_list->setSpacing(40);
        favorites_list->setIconSize(QSize(64, 64));
        favorites_list->sortItems();


        
        setLayout(layout_root);
        

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
        


        cb_profile_switch = new QComboBox(this);
        cb_profile_switch->addItem("Profile1");
        cb_profile_switch->addItem("Profile2");

        



        QHBoxLayout* layout_dir_buttons = new QHBoxLayout();
        layout_dir_buttons->addWidget(button_add_dir);
        layout_dir_buttons->addWidget(button_remove_dir);
        

        layout_settings->addWidget(label_app_theme);
        layout_settings->addWidget(chb_file_extension);
        layout_settings->addWidget(cb_theme);
        layout_settings->addWidget(cb_profile_switch);
        layout_settings->addWidget(label_directories);
        layout_settings->addWidget(directory_list);
        layout_settings->addLayout(layout_dir_buttons);
        layout_settings->addWidget(button_save_settings);



        QFont font("Arial", 18);

        // Input field for application searching
        text_input = new QLineEdit(this);
        text_input->setPlaceholderText("Search application");
        text_input->setFixedHeight(50);
        text_input->setFont(font);


        
        
        

        // Calculate most used applications
        UpdateMostUsedApplications();

        
        //search_suggestions << "MKVMerge";
        //search_suggestions << "sublime";
        //search_suggestions << "spek";

        completer = new QCompleter(popular_apps);
        text_input->setCompleter(completer);

        layout_all_apps->addWidget(text_input);
        





        current_list = applications_list;



        // List widget custom arguments
        list_widget->setViewMode(QListWidget::IconMode);
        list_widget->setMovement(QListView::Static);
        list_widget->setWordWrap(true);
        list_widget->setWrapping(true);
        list_widget->setItemAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
        list_widget->setSpacing(40);
        list_widget->setIconSize(QSize(64, 64));
        list_widget->sortItems();

        

        layout_favorites->addWidget(favorites_list);
        layout_all_apps->addWidget(list_widget);
        

        // Load last used profile
        LoadProfile();
        connect(cb_theme, &QComboBox::currentTextChanged, this, &ApplicationExplorer::UpdateUIPalette);
        connect(cb_profile_switch, &QComboBox::currentTextChanged, this, &ApplicationExplorer::LoadProfile);

        




        // CONNECTIONS
        //connect(this, &QListWidget::aboutToQuit, this, &ApplicationExplorer::cleanup);
        connect(list_widget, &QListWidget::itemActivated, this, &ApplicationExplorer::ExecuteApplication);
        connect(favorites_list, &QListWidget::itemActivated, this, &ApplicationExplorer::ExecuteApplication);
        
        

        connect(chb_file_extension, &QCheckBox::stateChanged, this, &ApplicationExplorer::UpdateListDelayed);
        // Delay before keyboard input triggers application search
        timer_search.setSingleShot(true);
        connect(&timer_search, &QTimer::timeout, this, &ApplicationExplorer::UpdateListDelayed);
        connect(text_input, &QLineEdit::textChanged, this, &ApplicationExplorer::OnTextChanged);

        setContextMenuPolicy(Qt::CustomContextMenu);
        connect(this, &QListWidget::customContextMenuRequested, this, [this] {
            ApplicationExplorer::ShowContextMenu(QCursor::pos(), list_widget);
            });
        connect(this, &QListWidget::customContextMenuRequested, this, [this] {
            ApplicationExplorer::ShowContextMenu(QCursor::pos(), favorites_list);
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
            UpdateListDelayed();
            });

        // Functionality to remove directories
        QObject::connect(button_remove_dir, &QPushButton::clicked, [this]() {
            qDeleteAll(directory_list->selectedItems());
            DirectoryListUpdated();
            UpdateListDelayed();
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



        // Handling controller navigator - UI communication


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

        DirectoryListUpdated();
        UpdateListDelayed();

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
        // application cleanup on execution ending
    }






    void ShowContextMenu(const QPoint& pos, QListWidget* list)
    {
        
        QPoint local_pos = list->mapFromGlobal(pos);

        // Get the index of the item at the local position
        QModelIndex index = list->indexAt(local_pos);
        
        // Check if the right-click occurred on an item
        if (!index.isValid())
        {
            return;
        }


        QListWidgetItem* item = list->currentItem();
        if (!item)
            return;

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
                removed_apps.append(item->data(Qt::UserRole).toString());
                delete item;
                for (QString ite : removed_apps)
                {
                    std::cout << ite.toStdString() << std::endl;
                }
            });


        
        bool found = false;

        for (int i = 0; i < favorites_list->count(); ++i) 
        {
            
            QListWidgetItem* currentItem = favorites_list->item(i);

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

                    int index = favorites_list->row(item);
                    QListWidgetItem* item_to_remove = favorites_list->takeItem(index);
                    delete item_to_remove;
                    favorites_list->update();

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
                    favorites_list->addItem(item2);

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
            //std::cout << file_path.toStdString() << std::endl;
            
            
            
            if (file_path.endsWith(".lnk", Qt::CaseInsensitive) || file_path.endsWith(".url", Qt::CaseInsensitive))
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
                        std::cout << "Process finished with exit code:" << exitCode << "and exit status:" << exitStatus;
                    });
                QObject::connect(process, &QProcess::errorOccurred, [](QProcess::ProcessError error) {
                    std::cout << "Error occurred:" << error;
                    });
            }



            usageFrequency[file_name]++;
            UpdateMostUsedApplications();
            
            
            


            //completer->setModel(new QStringListModel(search_suggestions, completer));
            completer = new QCompleter(popular_apps);
            text_input->setCompleter(nullptr);
            text_input->setCompleter(completer);
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
        //timer->setSingleShot(true);
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












int main(int argc, char* argv[]) 
{
    AllocateConsole("Debug console for Qt6");
    //qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));
    
    QApplication app(argc, argv);
    
    ApplicationExplorer explorer(app);
    
    

    VirtualKeyboard QKeyboard;
    QKeyboard.setWindowFlags(Qt::WindowCloseButtonHint | Qt::FramelessWindowHint);
    //QKeyboard.showMaximized();

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
    //explorer.show();

    //explorer.setFixedSize(QGuiApplication::primaryScreen()->availableGeometry().size());
    //explorer.showMaximized();


    return app.exec();
}








