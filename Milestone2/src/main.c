#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "../include/FileReader.h"
#include "../include/Queue.h"
#include "../include/Semaphores.h"
#include "../include/os_logic.h" // GUI will use this API

#define CODE_SEGMENT_OFFSET 9
#define MLFQ_LEVELS 4

extern sem_t userInputSemaphore;
extern sem_t userOutputSemaphore;
extern sem_t fileSemaphore;

char* lastLine;

void removeNewline(char* buffer)
{
	// Find the newline character (if any) and replace it with '\0'
	size_t length = strlen(buffer);
	if (length > 0 && buffer[length - 1] == '\n') {
		buffer[length - 1] = '\0';
	}
}
// Scheduler functions are now part of os_logic.h, no need for these forward declarations

void initMemory();
void initPCBs();
void initSemaphores();
void initQueues();
char* processToString(ProcessState state);
void displayMemory(Memory* mainMemory);
char* select_file_dialog(GtkWidget *parent_window);
void checkWaitingQueue();
 // Forward declarations for existing scheduler functions

 // Global variables
 GtkWidget *window;
 GtkWidget *dashboard_frame, *scheduler_control_frame, *resource_mgmt_frame;
 GtkWidget *memory_viewer_frame, *log_console_frame, *process_creation_frame;

 // Dashboard widgets
 GtkWidget *total_processes_label, *current_cycle_label, *algorithm_label;
 GtkWidget *process_list_view;
 GtkListStore *process_list_store;
 GtkWidget *ready_queue_view, *blocked_queue_view, *running_process_view;
 GtkListStore *ready_queue_store, *blocked_queue_store, *running_process_store;

 // Scheduler control widgets
 GtkWidget *algorithm_combo;
 GtkWidget *start_button, *stop_button, *reset_button, *step_button,*step5_button;
 GtkWidget *quantum_spin;

 // Resource management widgets
 GtkWidget *mutex_status_grid;
 GtkWidget *blocked_resources_view;
 GtkListStore *blocked_resources_store;

 // Memory viewer widgets
 GtkWidget *memory_grid;
 GtkWidget *memory_cells[60]; // Labels for memory cells

 // Log & Console widgets
 GtkWidget *execution_log_view;
 GtkTextBuffer *execution_log_buffer;
 GtkWidget *event_messages_view;
 GtkTextBuffer *event_messages_buffer;

 // Process creation widgets
 GtkWidget *add_process_button;
 GtkWidget *arrival_time_entries[10]; // Max 10 processes
 GtkWidget *process_files_combo[10];

 // Simulation state
 int cycle_count = 0;
 bool simulation_running = false;
 GMainLoop *main_loop = NULL;
 guint timeout_id = 0;

 // CSS Provider for styling
 GtkCssProvider *provider;

 // Global reference for the notebook for navigation
 GtkWidget *notebook;
 // Function declarations
 static void setup_dashboard(GtkWidget *container);
 static void setup_scheduler_control(GtkWidget *container);
 static void setup_resource_management(GtkWidget *container);
 static void setup_memory_viewer(GtkWidget *container);
 static void setup_log_console(GtkWidget *container);
 static void setup_process_creation(GtkWidget *container);

 static void update_dashboard();
 static void update_process_list();
 static void update_queues();
 static void update_resource_management();
 static void update_memory_viewer();
 static void add_execution_log(const char *message);
 static void add_event_message(const char *message);

 static void on_start_button_clicked(GtkButton *button, gpointer user_data);
 static void on_stop_button_clicked(GtkButton *button, gpointer user_data);
 static void on_reset_button_clicked(GtkButton *button, gpointer user_data);
 static void on_step_button_clicked(GtkButton *button, gpointer user_data);
static void on_step5_clicked(GtkButton *button, gpointer user_data);
 static void on_add_process_button_clicked(GtkButton *button, gpointer user_data);
 static void on_algorithm_changed(GtkComboBox *widget, gpointer user_data);
 static void on_browse_button_clicked(GtkButton *button, gpointer user_data);
 static gboolean simulation_step(gpointer user_data);
 char* get_user_input_from_dialog(int process_id);
 void show_text_dialog(const char *text);
// Improved CSS for the OS Scheduler Simulation
const char *css_data =
"window { background-color: #2d2d2d; color: #e0e0e0; }"
"notebook { background-color: #2d2d2d; }"
"notebook tab { background-color: #3d3d3d; color: #e0e0e0; padding: 8px; border-radius: 4px 4px 0 0; }"
"notebook tab:checked { background-color: #4d4d4d; font-weight: bold; }"
"frame { background-color: #3d3d3d; border-radius: 5px; border: 1px solid #555; margin: 10px; padding: 10px; }"
"frame > label { font-weight: bold; color: #e0e0e0; }"
"button { background-color: #0078d7; color: white; border-radius: 4px; padding: 6px 12px; border: none; }"
"button:hover { background-color: #1a88e1; }"
"label { color: #e0e0e0; }"
"entry { background-color: #4d4d4d; color: #e0e0e0; border: 1px solid #555; border-radius: 3px; padding: 5px; }"
".running { background-color: #3c9f40; color: white; padding: 4px; border-radius: 3px; }"
".ready { background-color: #0078d7; color: white; padding: 4px; border-radius: 3px; }"
".blocked { background-color: #d83b01; color: white; padding: 4px; border-radius: 3px; }"
".memory-cell { border: 1px solid #555; padding: 4px; font-family: monospace; background-color: #3d3d3d; color: #e0e0e0; }"
".memory-cell-used { background-color: #264f78; }";

//new css
// Complete CSS for the OS Scheduler Simulation with enforced dark theme across all components
// const char *css_data = 
// "@define-color bg_color #2d2d2d;"
// "@define-color fg_color #e0e0e0;"
// "@define-color base_color #3d3d3d;"
// "@define-color text_color #e0e0e0;"
// "@define-color selected_bg_color #0078d7;"
// "@define-color selected_fg_color white;"
// "@define-color tooltip_bg_color #2d2d2d;"
// "@define-color tooltip_fg_color #e0e0e0;"

// /* Basic theme colors for all widgets */
// "* {"
// "    color: @fg_color;"
// "    border-color: #555;"
// "    background-color: @bg_color;"
// "}"

// /* Window and general containers */
// "window, dialog {"
// "    background-color: @bg_color;"
// "    color: @fg_color;"
// "}"

// /* Notebook (tabbed interface) styling */
// "notebook {"
// "    background-color: @bg_color;"
// "    color: @fg_color;"
// "}"

// "notebook header {"
// "    background-color: @bg_color;"
// "}"

// "notebook tab {"
// "    background-color: #3d3d3d;"
// "    color: @fg_color;"
// "    padding: 8px;"
// "    border-radius: 4px 4px 0 0;"
// "    border: 1px solid #555;"
// "    margin: 1px;"
// "}"

// "notebook tab:checked {"
// "    background-color: #4d4d4d;"
// "    font-weight: bold;"
// "}"

// /* Frame styling */
// "frame {"
// "    background-color: #3d3d3d;"
// "    border-radius: 5px;"
// "    border: 1px solid #555;"
// "    margin: 10px;"
// "    padding: 10px;"
// "}"

// "frame > label {"
// "    font-weight: bold;"
// "    color: @fg_color;"
// "}"

// /* Buttons styling */
// "button {"
// "    background-color: #0078d7;"
// "    color: white;"
// "    border-radius: 4px;"
// "    padding: 6px 12px;"
// "    border: none;"
// "}"

// "button:hover {"
// "    background-color: #1a88e1;"
// "}"

// "button:active {"
// "    background-color: #005fa3;"
// "}"

// "button:disabled {"
// "    background-color: #4d4d4d;"
// "    color: #aaa;"
// "}"

// /* ComboBox styling */
// "combobox {"
// "    background-color: #4d4d4d;"
// "    color: @fg_color;"
// "    border: 1px solid #555;"
// "    border-radius: 3px;"
// "}"

// "combobox button {"
// "    background-color: #4d4d4d;"
// "    color: @fg_color;"
// "}"

// "combobox entry {"
// "    background-color: #4d4d4d;"
// "    color: @fg_color;"
// "}"

// "combobox menu {"
// "    background-color: #3d3d3d;"
// "}"

// "combobox menuitem {"
// "    background-color: #3d3d3d;"
// "    color: @fg_color;"
// "}"

// "combobox menuitem:hover {"
// "    background-color: @selected_bg_color;"
// "    color: @selected_fg_color;"
// "}"

// "combobox box {"
// "    background-color: #4d4d4d;"
// "}"

// "popover {"
// "    background-color: #3d3d3d;"
// "    color: @fg_color;"
// "}"

// /* TreeView styling (for lists) */
// "treeview {"
// "    background-color: #3d3d3d;"
// "    color: @fg_color;"
// "    border: 1px solid #555;"
// "}"

// "treeview:selected {"
// "    background-color: @selected_bg_color;"
// "    color: @selected_fg_color;"
// "}"

// "treeview header {"
// "    background-color: #4d4d4d;"
// "    color: @fg_color;"
// "}"

// "treeview header button {"
// "    background-color: #4d4d4d;"
// "    color: @fg_color;"
// "    border: 1px solid #555;"
// "}"

// /* ScrolledWindow styling */
// "scrolledwindow {"
// "    background-color: @bg_color;"
// "    border: 1px solid #555;"
// "}"

// /* TextView styling (for logs) */
// "textview {"
// "    background-color: #3d3d3d;"
// "    color: @fg_color;"
// "}"

// "textview text {"
// "    background-color: #3d3d3d;"
// "    color: @fg_color;"
// "}"

// /* Entry styling */
// "entry {"
// "    background-color: #4d4d4d;"
// "    color: @fg_color;"
// "    border: 1px solid #555;"
// "    border-radius: 3px;"
// "    padding: 5px;"
// "}"

// /* SpinButton styling */
// "spinbutton {"
// "    background-color: #4d4d4d;"
// "    color: @fg_color;"
// "}"

// "spinbutton entry {"
// "    background-color: #4d4d4d;"
// "    color: @fg_color;"
// "}"

// "spinbutton button {"
// "    background-color: #4d4d4d;"
// "    color: @fg_color;"
// "}"

// /* Custom classes */
// ".running {"
// "    background-color: #3c9f40;"
// "    color: white;"
// "    padding: 4px;"
// "    border-radius: 3px;"
// "}"

// ".ready {"
// "    background-color: #0078d7;"
// "    color: white;"
// "    padding: 4px;"
// "    border-radius: 3px;"
// "}"

// ".blocked {"
// "    background-color: #d83b01;"
// "    color: white;"
// "    padding: 4px;"
// "    border-radius: 3px;"
// "}"

// ".memory-cell {"
// "    border: 1px solid #555;"
// "    padding: 4px;"
// "    font-family: monospace;"
// "    background-color: #3d3d3d;"
// "    color: @fg_color;"
// "}"

// ".memory-cell-used {"
// "    background-color: #264f78;"
// "}";
//new css end


int main(int argc, char *argv[]) {
    // Initialize GTK
    gtk_init(&argc, &argv);

    // Create the main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "OS Scheduler Simulation");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Apply CSS styling
    provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, css_data, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    // Create main layout container
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), main_box);

    // Create notebook (tabbed interface)
    notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(main_box), notebook, TRUE, TRUE, 0);

    // Tab 1: Dashboard & Controls
    GtkWidget *tab1_content = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(tab1_content), 10);
    gtk_grid_set_column_spacing(GTK_GRID(tab1_content), 10);
    gtk_container_set_border_width(GTK_CONTAINER(tab1_content), 15);

    // Create frames for dashboard tab
    dashboard_frame = gtk_frame_new("Main Dashboard");
    scheduler_control_frame = gtk_frame_new("Scheduler Control Panel");
    resource_mgmt_frame = gtk_frame_new("Resource Management Panel");

    // Set frames to expand and fill available space
    gtk_widget_set_hexpand(dashboard_frame, TRUE);
    gtk_widget_set_vexpand(dashboard_frame, TRUE);
    gtk_widget_set_hexpand(scheduler_control_frame, TRUE);
    gtk_widget_set_vexpand(scheduler_control_frame, TRUE);
    gtk_widget_set_hexpand(resource_mgmt_frame, TRUE);
    gtk_widget_set_vexpand(resource_mgmt_frame, TRUE);

    // Layout the frames in the grid for tab 1
    gtk_grid_attach(GTK_GRID(tab1_content), dashboard_frame, 0, 0, 2, 2);
    gtk_grid_attach(GTK_GRID(tab1_content), scheduler_control_frame, 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(tab1_content), resource_mgmt_frame, 2, 1, 1, 1);

    GtkWidget *tab1_label = gtk_label_new("Dashboard");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab1_content, tab1_label);

    // Tab 2: Memory Viewer
    GtkWidget *tab2_content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(tab2_content), 15);

    memory_viewer_frame = gtk_frame_new("Memory Viewer");
    gtk_widget_set_hexpand(memory_viewer_frame, TRUE);
    gtk_widget_set_vexpand(memory_viewer_frame, TRUE);
    gtk_box_pack_start(GTK_BOX(tab2_content), memory_viewer_frame, TRUE, TRUE, 0);

    GtkWidget *tab2_label = gtk_label_new("Memory");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab2_content, tab2_label);

    // Tab 3: Process Creation
    GtkWidget *tab3_content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(tab3_content), 15);

    process_creation_frame = gtk_frame_new("Process Creation and Configuration");
    gtk_widget_set_hexpand(process_creation_frame, TRUE);
    gtk_widget_set_vexpand(process_creation_frame, TRUE);
    gtk_box_pack_start(GTK_BOX(tab3_content), process_creation_frame, TRUE, TRUE, 0);

    GtkWidget *tab3_label = gtk_label_new("Process Creation");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab3_content, tab3_label);

    // Tab 4: Logs and Console
    GtkWidget *tab4_content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(tab4_content), 15);

    log_console_frame = gtk_frame_new("Log & Console Panel");
    gtk_widget_set_hexpand(log_console_frame, TRUE);
    gtk_widget_set_vexpand(log_console_frame, TRUE);
    gtk_box_pack_start(GTK_BOX(tab4_content), log_console_frame, TRUE, TRUE, 0);

    GtkWidget *tab4_label = gtk_label_new("Logs");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab4_content, tab4_label);

    // Setup the content for each frame
    setup_dashboard(dashboard_frame);
    setup_scheduler_control(scheduler_control_frame);
    setup_resource_management(resource_mgmt_frame);
    setup_memory_viewer(memory_viewer_frame);
    setup_log_console(log_console_frame);
    setup_process_creation(process_creation_frame);

    // Initialize the simulation logic
    os_init();

    // Show all widgets
    gtk_widget_show_all(window);

    // Start the GTK main loop
    gtk_main();
    return 0;
}

 static void setup_dashboard(GtkWidget *container) {
     GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
     gtk_container_add(GTK_CONTAINER(container), box);
     gtk_container_set_border_width(GTK_CONTAINER(box), 10);

     // Overview Section
     GtkWidget *overview_frame = gtk_frame_new("Overview");
     gtk_box_pack_start(GTK_BOX(box), overview_frame, FALSE, FALSE, 0);

     GtkWidget *overview_grid = gtk_grid_new();
     gtk_container_add(GTK_CONTAINER(overview_frame), overview_grid);
     gtk_grid_set_row_spacing(GTK_GRID(overview_grid), 5);
     gtk_grid_set_column_spacing(GTK_GRID(overview_grid), 5);
     gtk_container_set_border_width(GTK_CONTAINER(overview_grid), 5);

     // Total Processes
     GtkWidget *total_processes_title = gtk_label_new("Total Processes:");
     gtk_grid_attach(GTK_GRID(overview_grid), total_processes_title, 0, 0, 1, 1);

     total_processes_label = gtk_label_new("0");
     gtk_grid_attach(GTK_GRID(overview_grid), total_processes_label, 1, 0, 1, 1);

     // Current Cycle
     GtkWidget *current_cycle_title = gtk_label_new("Upcoming Cycle:");
     gtk_grid_attach(GTK_GRID(overview_grid), current_cycle_title, 0, 1, 1, 1);

     current_cycle_label = gtk_label_new("0");
     gtk_grid_attach(GTK_GRID(overview_grid), current_cycle_label, 1, 1, 1, 1);

     // Active Algorithm
     GtkWidget *algorithm_title = gtk_label_new("Active Algorithm:");
     gtk_grid_attach(GTK_GRID(overview_grid), algorithm_title, 0, 2, 1, 1);

     algorithm_label = gtk_label_new("FCFS");
     gtk_grid_attach(GTK_GRID(overview_grid), algorithm_label, 1, 2, 1, 1);

     // Process List
     GtkWidget *process_list_frame = gtk_frame_new("Process List");
     gtk_box_pack_start(GTK_BOX(box), process_list_frame, TRUE, TRUE, 0);

     // Create a scrolled window for the process list
     GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
     gtk_container_add(GTK_CONTAINER(process_list_frame), scrolled_window);
     gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

     // Create the tree view for processes
     process_list_store = gtk_list_store_new(6, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT,
                                              G_TYPE_INT, G_TYPE_INT, G_TYPE_INT);

     process_list_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(process_list_store));
     gtk_container_add(GTK_CONTAINER(scrolled_window), process_list_view);

     // Add columns to the tree view
     GtkCellRenderer *renderer = gtk_cell_renderer_text_new();

     GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("PID", renderer,
                                                                        "text", 0, NULL);
     gtk_tree_view_append_column(GTK_TREE_VIEW(process_list_view), column);

     column = gtk_tree_view_column_new_with_attributes("State", renderer,
                                                     "text", 1, NULL);
     gtk_tree_view_append_column(GTK_TREE_VIEW(process_list_view), column);

     column = gtk_tree_view_column_new_with_attributes("Priority", renderer,
                                                     "text", 2, NULL);
     gtk_tree_view_append_column(GTK_TREE_VIEW(process_list_view), column);

     column = gtk_tree_view_column_new_with_attributes("Mem Start", renderer,
                                                     "text", 3, NULL);
     gtk_tree_view_append_column(GTK_TREE_VIEW(process_list_view), column);

     column = gtk_tree_view_column_new_with_attributes("Mem End", renderer,
                                                     "text", 4, NULL);
     gtk_tree_view_append_column(GTK_TREE_VIEW(process_list_view), column);

     column = gtk_tree_view_column_new_with_attributes("PC", renderer,
                                                     "text", 5, NULL);
     gtk_tree_view_append_column(GTK_TREE_VIEW(process_list_view), column);

     // Queue Section
     GtkWidget *queue_notebook = gtk_notebook_new();
     gtk_box_pack_start(GTK_BOX(box), queue_notebook, TRUE, TRUE, 0);

     // Ready Queue
     GtkWidget *ready_queue_scrolled = gtk_scrolled_window_new(NULL, NULL);
     gtk_notebook_append_page(GTK_NOTEBOOK(queue_notebook), ready_queue_scrolled,
                            gtk_label_new("Ready Queue"));

     ready_queue_store = gtk_list_store_new(3, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT);
     ready_queue_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(ready_queue_store));
     gtk_container_add(GTK_CONTAINER(ready_queue_scrolled), ready_queue_view);

     column = gtk_tree_view_column_new_with_attributes("PID", renderer, "text", 0, NULL);
     gtk_tree_view_append_column(GTK_TREE_VIEW(ready_queue_view), column);

     column = gtk_tree_view_column_new_with_attributes("Instruction", renderer, "text", 1, NULL);
     gtk_tree_view_append_column(GTK_TREE_VIEW(ready_queue_view), column);

     column = gtk_tree_view_column_new_with_attributes("Time in Queue", renderer, "text", 2, NULL);
     gtk_tree_view_append_column(GTK_TREE_VIEW(ready_queue_view), column);

     // Blocked Queue
     GtkWidget *blocked_queue_scrolled = gtk_scrolled_window_new(NULL, NULL);
     gtk_notebook_append_page(GTK_NOTEBOOK(queue_notebook), blocked_queue_scrolled,
                            gtk_label_new("Blocked Queue"));

     blocked_queue_store = gtk_list_store_new(3, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT);
     blocked_queue_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(blocked_queue_store));
     gtk_container_add(GTK_CONTAINER(blocked_queue_scrolled), blocked_queue_view);

     column = gtk_tree_view_column_new_with_attributes("PID", renderer, "text", 0, NULL);
     gtk_tree_view_append_column(GTK_TREE_VIEW(blocked_queue_view), column);

     column = gtk_tree_view_column_new_with_attributes("Resource", renderer, "text", 1, NULL);
     gtk_tree_view_append_column(GTK_TREE_VIEW(blocked_queue_view), column);

     column = gtk_tree_view_column_new_with_attributes("Time Blocked", renderer, "text", 2, NULL);
     gtk_tree_view_append_column(GTK_TREE_VIEW(blocked_queue_view), column);

     // Running Process
     GtkWidget *running_process_scrolled = gtk_scrolled_window_new(NULL, NULL);
     gtk_notebook_append_page(GTK_NOTEBOOK(queue_notebook), running_process_scrolled,
                            gtk_label_new("Running Process"));

     running_process_store = gtk_list_store_new(3, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT);
     running_process_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(running_process_store));
     gtk_container_add(GTK_CONTAINER(running_process_scrolled), running_process_view);

     column = gtk_tree_view_column_new_with_attributes("PID", renderer, "text", 0, NULL);
     gtk_tree_view_append_column(GTK_TREE_VIEW(running_process_view), column);

     column = gtk_tree_view_column_new_with_attributes("Current Instruction", renderer, "text", 1, NULL);
     gtk_tree_view_append_column(GTK_TREE_VIEW(running_process_view), column);

     column = gtk_tree_view_column_new_with_attributes("Time Running", renderer, "text", 2, NULL);
     gtk_tree_view_append_column(GTK_TREE_VIEW(running_process_view), column);
 }

 static void setup_scheduler_control(GtkWidget *container) {
     GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
     gtk_container_add(GTK_CONTAINER(container), box);
     gtk_container_set_border_width(GTK_CONTAINER(box), 10);

     // Algorithm selection
     GtkWidget *algorithm_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
     gtk_box_pack_start(GTK_BOX(box), algorithm_box, FALSE, FALSE, 0);

     GtkWidget *algorithm_label = gtk_label_new("Select Algorithm:");
     gtk_box_pack_start(GTK_BOX(algorithm_box), algorithm_label, FALSE, FALSE, 0);

     algorithm_combo = gtk_combo_box_text_new();
     gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algorithm_combo), "First Come First Serve");
     gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algorithm_combo), "Round Robin");
     gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algorithm_combo), "Multilevel Feedback Queue");
     gtk_combo_box_set_active(GTK_COMBO_BOX(algorithm_combo), 0);
     gtk_box_pack_start(GTK_BOX(algorithm_box), algorithm_combo, TRUE, TRUE, 0);
     g_signal_connect(algorithm_combo, "changed", G_CALLBACK(on_algorithm_changed), NULL);

     // Quantum setting
     GtkWidget *quantum_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
     gtk_box_pack_start(GTK_BOX(box), quantum_box, FALSE, FALSE, 0);

     GtkWidget *quantum_label = gtk_label_new("RR Quantum:");
     gtk_box_pack_start(GTK_BOX(quantum_box), quantum_label, FALSE, FALSE, 0);

     quantum_spin = gtk_spin_button_new_with_range(1, 10, 1);
     gtk_box_pack_start(GTK_BOX(quantum_box), quantum_spin, TRUE, TRUE, 0);
     gtk_widget_set_sensitive(quantum_spin, FALSE);  // Initially disabled for FCFS

     // Control buttons
     GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
     gtk_box_pack_start(GTK_BOX(box), button_box, FALSE, FALSE, 0);

     start_button = gtk_button_new_with_label("Start");
     gtk_box_pack_start(GTK_BOX(button_box), start_button, TRUE, TRUE, 0);
     g_signal_connect(start_button, "clicked", G_CALLBACK(on_start_button_clicked), NULL);

     stop_button = gtk_button_new_with_label("Stop");
     gtk_box_pack_start(GTK_BOX(button_box), stop_button, TRUE, TRUE, 0);
     gtk_widget_set_sensitive(stop_button, FALSE);  // Initially disabled
     g_signal_connect(stop_button, "clicked", G_CALLBACK(on_stop_button_clicked), NULL);

     reset_button = gtk_button_new_with_label("Reset");
     gtk_box_pack_start(GTK_BOX(button_box), reset_button, TRUE, TRUE, 0);
     g_signal_connect(reset_button, "clicked", G_CALLBACK(on_reset_button_clicked), NULL);

     step_button = gtk_button_new_with_label("Step");
     gtk_box_pack_start(GTK_BOX(box), step_button, FALSE, FALSE, 0);
     g_signal_connect(step_button, "clicked", G_CALLBACK(on_step_button_clicked), NULL);

	step5_button = gtk_button_new_with_label("Execute 5 Instructions");
	gtk_box_pack_start(GTK_BOX(box), step5_button, FALSE, FALSE, 0);
	g_signal_connect(step5_button, "clicked", G_CALLBACK(on_step5_clicked), NULL);
 }

 static void setup_resource_management(GtkWidget *container) {
     GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
     gtk_container_add(GTK_CONTAINER(container), box);
     gtk_container_set_border_width(GTK_CONTAINER(box), 10);

     // Mutex Status
     GtkWidget *mutex_frame = gtk_frame_new("Mutex Status");
     gtk_box_pack_start(GTK_BOX(box), mutex_frame, FALSE, FALSE, 0);

     mutex_status_grid = gtk_grid_new();
     gtk_container_add(GTK_CONTAINER(mutex_frame), mutex_status_grid);
     gtk_grid_set_row_spacing(GTK_GRID(mutex_status_grid), 5);
     gtk_grid_set_column_spacing(GTK_GRID(mutex_status_grid), 5);
     gtk_container_set_border_width(GTK_CONTAINER(mutex_status_grid), 5);

     // Resource headers
     GtkWidget *resource_header = gtk_label_new("Resource");
     gtk_grid_attach(GTK_GRID(mutex_status_grid), resource_header, 0, 0, 1, 1);

     GtkWidget *status_header = gtk_label_new("Status");
     gtk_grid_attach(GTK_GRID(mutex_status_grid), status_header, 1, 0, 1, 1);

     GtkWidget *holder_header = gtk_label_new("Holder/Waiter");
     gtk_grid_attach(GTK_GRID(mutex_status_grid), holder_header, 2, 0, 1, 1);

     // User Input
     GtkWidget *user_input_label = gtk_label_new("userInput");
     gtk_grid_attach(GTK_GRID(mutex_status_grid), user_input_label, 0, 1, 1, 1);

     GtkWidget *user_input_status = gtk_label_new("Free");
     gtk_grid_attach(GTK_GRID(mutex_status_grid), user_input_status, 1, 1, 1, 1);

     GtkWidget *user_input_holder = gtk_label_new("None");
     gtk_grid_attach(GTK_GRID(mutex_status_grid), user_input_holder, 2, 1, 1, 1);

     // User Output
     GtkWidget *user_output_label = gtk_label_new("userOutput");
     gtk_grid_attach(GTK_GRID(mutex_status_grid), user_output_label, 0, 2, 1, 1);

     GtkWidget *user_output_status = gtk_label_new("Free");
     gtk_grid_attach(GTK_GRID(mutex_status_grid), user_output_status, 1, 2, 1, 1);

     GtkWidget *user_output_holder = gtk_label_new("None");
     gtk_grid_attach(GTK_GRID(mutex_status_grid), user_output_holder, 2, 2, 1, 1);

     // File
     GtkWidget *file_label = gtk_label_new("file");
     gtk_grid_attach(GTK_GRID(mutex_status_grid), file_label, 0, 3, 1, 1);

     GtkWidget *file_status = gtk_label_new("Free");
     gtk_grid_attach(GTK_GRID(mutex_status_grid), file_status, 1, 3, 1, 1);

     GtkWidget *file_holder = gtk_label_new("None");
     gtk_grid_attach(GTK_GRID(mutex_status_grid), file_holder, 2, 3, 1, 1);

     // Blocked Queue for Resources
     GtkWidget *blocked_resources_frame = gtk_frame_new("Blocked Queue for Resources");
     gtk_box_pack_start(GTK_BOX(box), blocked_resources_frame, TRUE, TRUE, 0);

     GtkWidget *blocked_resources_scroll = gtk_scrolled_window_new(NULL, NULL);
     gtk_container_add(GTK_CONTAINER(blocked_resources_frame), blocked_resources_scroll);

     blocked_resources_store = gtk_list_store_new(3, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT);
     blocked_resources_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(blocked_resources_store));
     gtk_container_add(GTK_CONTAINER(blocked_resources_scroll), blocked_resources_view);

     GtkCellRenderer *renderer = gtk_cell_renderer_text_new();

     GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("PID", renderer,
                                                                        "text", 0, NULL);
     gtk_tree_view_append_column(GTK_TREE_VIEW(blocked_resources_view), column);

     column = gtk_tree_view_column_new_with_attributes("Resource", renderer,
                                                     "text", 1, NULL);
     gtk_tree_view_append_column(GTK_TREE_VIEW(blocked_resources_view), column);

     column = gtk_tree_view_column_new_with_attributes("Priority", renderer,
                                                     "text", 2, NULL);
     gtk_tree_view_append_column(GTK_TREE_VIEW(blocked_resources_view), column);
 }

 static void setup_memory_viewer(GtkWidget *container) {
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(container), scrolled_window);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    memory_grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(scrolled_window), memory_grid);
    gtk_grid_set_row_spacing(GTK_GRID(memory_grid), 1);
    gtk_grid_set_column_spacing(GTK_GRID(memory_grid), 1);
    gtk_container_set_border_width(GTK_CONTAINER(memory_grid), 5);

    // Header
    GtkWidget *header_index = gtk_label_new("Index");
    gtk_grid_attach(GTK_GRID(memory_grid), header_index, 0, 0, 1, 1);

    GtkWidget *header_name = gtk_label_new("Name");
    gtk_grid_attach(GTK_GRID(memory_grid), header_name, 1, 0, 1, 1);

    GtkWidget *header_data = gtk_label_new("Data");
    gtk_grid_attach(GTK_GRID(memory_grid), header_data, 2, 0, 1, 1);

    // Memory cells
    for (int i = 0; i < 60; i++) {
        // Index label
        char index_str[10];
        sprintf(index_str, "%d", i);
        GtkWidget *index_label = gtk_label_new(index_str);
        gtk_grid_attach(GTK_GRID(memory_grid), index_label, 0, i + 1, 1, 1);

        // Create a box to hold name and data
        memory_cells[i] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

        // Name label
        GtkWidget *name_label = gtk_label_new("");
        gtk_label_set_xalign(GTK_LABEL(name_label), 0.0);
        gtk_box_pack_start(GTK_BOX(memory_cells[i]), name_label, FALSE, FALSE, 5);

        // Data label
        GtkWidget *data_label = gtk_label_new("");
        gtk_label_set_xalign(GTK_LABEL(data_label), 0.0);
        gtk_label_set_ellipsize(GTK_LABEL(data_label), PANGO_ELLIPSIZE_END);
        gtk_widget_set_size_request(data_label, 300, -1);
        gtk_box_pack_start(GTK_BOX(memory_cells[i]), data_label, TRUE, TRUE, 5);

        // Now ATTACH the box itself
        gtk_grid_attach(GTK_GRID(memory_grid), memory_cells[i], 1, i + 1, 2, 1);

        // CSS classes
        GtkStyleContext *context = gtk_widget_get_style_context(name_label);
        gtk_style_context_add_class(context, "memory-cell");

        context = gtk_widget_get_style_context(data_label);
        gtk_style_context_add_class(context, "memory-cell");
    }
}

 static void setup_log_console(GtkWidget *container) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(container), box);
    gtk_container_set_border_width(GTK_CONTAINER(box), 10);

    // Create a notebook for the log and console
    GtkWidget *notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(box), notebook, TRUE, TRUE, 0);

    // Execution Log Tab
    GtkWidget *exec_log_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), exec_log_scroll,
                           gtk_label_new("Execution Log"));

    execution_log_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(execution_log_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(execution_log_view), GTK_WRAP_WORD_CHAR);
    execution_log_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(execution_log_view));
    gtk_container_add(GTK_CONTAINER(exec_log_scroll), execution_log_view);

    // Event Messages Tab
    GtkWidget *event_msg_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), event_msg_scroll,
                           gtk_label_new("Event Messages"));

    event_messages_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(event_messages_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(event_messages_view), GTK_WRAP_WORD_CHAR);
    event_messages_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(event_messages_view));
    gtk_container_add(GTK_CONTAINER(event_msg_scroll), event_messages_view);
}

// Add a "Browse" button to select files from filesystem
static void setup_process_creation(GtkWidget *container) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(container), box);
    gtk_container_set_border_width(GTK_CONTAINER(box), 10);

    // Load process section
    GtkWidget *load_process_frame = gtk_frame_new("Load Processes");
    gtk_box_pack_start(GTK_BOX(box), load_process_frame, FALSE, FALSE, 0);

    GtkWidget *load_process_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(load_process_frame), load_process_box);
    gtk_container_set_border_width(GTK_CONTAINER(load_process_box), 5);

    // Create entries for 10 processes
    for (int i = 0; i < 10; i++) {
        GtkWidget *process_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
        gtk_box_pack_start(GTK_BOX(load_process_box), process_box, FALSE, FALSE, 2);

        char label_text[20];
        sprintf(label_text, "Process %d:", i + 1);
        GtkWidget *label = gtk_label_new(label_text);
        gtk_box_pack_start(GTK_BOX(process_box), label, FALSE, FALSE, 0);

        process_files_combo[i] = gtk_combo_box_text_new_with_entry();
        gtk_box_pack_start(GTK_BOX(process_box), process_files_combo[i], TRUE, TRUE, 0);

        // Add a browse button for each process
        GtkWidget *browse_button = gtk_button_new_with_label("Browse...");
        gtk_box_pack_start(GTK_BOX(process_box), browse_button, FALSE, FALSE, 0);
        g_signal_connect(browse_button, "clicked", G_CALLBACK(on_browse_button_clicked), GINT_TO_POINTER(i));

        GtkWidget *arrival_label = gtk_label_new("Arrival Time:");
        gtk_box_pack_start(GTK_BOX(process_box), arrival_label, FALSE, FALSE, 0);

        arrival_time_entries[i] = gtk_spin_button_new_with_range(0, 100, 1);
        gtk_box_pack_start(GTK_BOX(process_box), arrival_time_entries[i], FALSE, FALSE, 0);
    }

    // Add process button
    add_process_button = gtk_button_new_with_label("Add Selected Processes");
    gtk_box_pack_start(GTK_BOX(load_process_box), add_process_button, FALSE, FALSE, 5);
    g_signal_connect(add_process_button, "clicked", G_CALLBACK(on_add_process_button_clicked), NULL);
}

static void on_browse_button_clicked(GtkButton *button, gpointer user_data) {
    int index = GPOINTER_TO_INT(user_data);
    GtkWidget *parent_window = gtk_widget_get_toplevel(GTK_WIDGET(button));

    GtkWidget *dialog = gtk_file_chooser_dialog_new("Select Process File",
                                                   GTK_WINDOW(parent_window),
                                                   GTK_FILE_CHOOSER_ACTION_OPEN,
                                                   "Cancel", GTK_RESPONSE_CANCEL,
                                                   "Open", GTK_RESPONSE_ACCEPT,
                                                   NULL);

    // Set up filter for text files
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Text Files");
    gtk_file_filter_add_pattern(filter, "*.txt");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    // Add filter for all files
    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "All Files");
    gtk_file_filter_add_pattern(filter, "*");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

        // Set the selected file in the combo box entry
        GtkWidget *entry = gtk_bin_get_child(GTK_BIN(process_files_combo[index]));
        gtk_entry_set_text(GTK_ENTRY(entry), filename);

        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

static void update_dashboard() {
    OSStatus status;
    os_get_status(&status);

    char buffer[256];
    sprintf(buffer, "Total Processes: %d", status.total_processes);
    gtk_label_set_text(GTK_LABEL(total_processes_label), buffer);

    sprintf(buffer, "Current Cycle: %d", status.current_cycle);
    gtk_label_set_text(GTK_LABEL(current_cycle_label), buffer);
    cycle_count = status.current_cycle; // Update local cycle_count if still needed by GUI logic

    const char* scheduler_name;
    switch (status.scheduler_type) {
        case SCHED_FCFS: scheduler_name = "FCFS"; break;
        case SCHED_ROUND_ROBIN: scheduler_name = "Round Robin"; break;
        case SCHED_MLFQ: scheduler_name = "MLFQ"; break;
        default: scheduler_name = "Unknown"; break;
    }
    sprintf(buffer, "Algorithm: %s", scheduler_name);
    gtk_label_set_text(GTK_LABEL(algorithm_label), buffer);

    update_process_list();
    update_queues();
}

char* get_user_input_from_dialog(int process_id) {
    static char input_value[256];
    GtkWidget *dialog, *content_area, *entry, *label;
    GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

    // Create dialog
    char title[64];
    sprintf(title, "Process %d Input", process_id);
    dialog = gtk_dialog_new_with_buttons(title,
                                        GTK_WINDOW(window),
                                        flags,
                                        "OK", GTK_RESPONSE_ACCEPT,
                                        NULL);

    // Add content
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    label = gtk_label_new("Please enter a value:");
    entry = gtk_entry_new();

    gtk_container_add(GTK_CONTAINER(content_area), label);
    gtk_container_add(GTK_CONTAINER(content_area), entry);
    gtk_widget_show_all(dialog);

    // Run dialog and get result
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        const char* text = gtk_entry_get_text(GTK_ENTRY(entry));
        strncpy(input_value, text, 255);
        input_value[255] = '\0';
    } else {
        // Dialog was canceled, provide default value
        strcpy(input_value, "");
    }

    gtk_widget_destroy(dialog);
    return input_value;
}

void show_text_dialog(const char *text) {
	GtkWidget *dialog, *content_area, *label;
	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
	char *token;
	char *text_copy = strdup(text);  // Make a copy since strtok modifies the string
	char display_text[1024] = "";    // Buffer for formatted output
	int count = 0;

	// Split the text on spaces and format with 10 words per line
	token = strtok(text_copy, " ");
	while (token != NULL) {
		// Add the token to the display text
		strcat(display_text, token);
		strcat(display_text, " ");  // Add space between words

		count++;
		if (count % 10 == 0) {  // After every 10 words
			strcat(display_text, "\n");  // Add a newline
		}
		token = strtok(NULL, " ");
	}

	// Create dialog
	dialog = gtk_dialog_new_with_buttons("Message",
										GTK_WINDOW(window),
										flags,
										"OK", GTK_RESPONSE_ACCEPT,
										NULL);

	// Add content
	content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	label = gtk_label_new(display_text);
	gtk_container_add(GTK_CONTAINER(content_area), label);
	gtk_widget_show_all(dialog);

	// Wait for user to press OK
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	// Free the duplicated string
	free(text_copy);
}

static void update_process_list() {
    gtk_list_store_clear(process_list_store);
    ProcessControlBlock* processes[MAX_PROCESSES];
    int count;
    os_get_all_processes(processes, &count);

    GtkTreeIter iter;
    for (int i = 0; i < count; ++i) {
        gtk_list_store_append(process_list_store, &iter);
        char state_str[50];
        strcpy(state_str, os_process_state_str(processes[i]->state));
        gtk_list_store_set(process_list_store, &iter,
                           0, processes[i]->id,
                           1, state_str, // Use API to convert state to string
                           2, processes[i]->priority,
                           3, processes[i]->program_counter,
                           4, processes[i]->memory_start,
                           5, processes[i]->memory_end,
                           -1);
    }
}

static void update_queues() {
    gtk_list_store_clear(ready_queue_store);
    gtk_list_store_clear(blocked_queue_store);
    gtk_list_store_clear(running_process_store);

    GtkTreeIter iter;
    OSStatus status;
    os_get_status(&status);

    // Running Process
    if (status.running_process) {
        gtk_list_store_append(running_process_store, &iter);
        char state_str[50];
        strcpy(state_str, os_process_state_str(status.running_process->state));
        gtk_list_store_set(running_process_store, &iter,
                           0, status.running_process->id,
                           1, state_str,
                           2, status.running_process->priority,
                           -1);
    }

    // Ready Queues
    Queue* ready_queues[MLFQ_LEVELS];
    os_get_ready_queue(ready_queues);
    for (int i = 0; i < MLFQ_LEVELS; ++i) {
        if (ready_queues[i]) {
            Node* current = ready_queues[i]->head;
            while (current != NULL) {
                ProcessControlBlock* pcb = (ProcessControlBlock*)current->data;
                if (pcb) {
                    gtk_list_store_append(ready_queue_store, &iter);
                    char state_str[50];
                    strcpy(state_str, os_process_state_str(pcb->state));
                    gtk_list_store_set(ready_queue_store, &iter,
                                       0, pcb->id,
                                       1, state_str,
                                       2, pcb->priority,
                                       -1);
                }
                current = current->next;
            }
        }
    }

    // Blocked Queue
    Queue blocked_q_val;
    os_get_blocked_queue(&blocked_q_val); // os_get_blocked_queue now takes a pointer to a Queue
    Node* current_blocked = blocked_q_val.head;
    while (current_blocked != NULL) {
        ProcessControlBlock* pcb = (ProcessControlBlock*)current_blocked->data;
        if (pcb) {
            gtk_list_store_append(blocked_queue_store, &iter);
            char state_str[50];
            strcpy(state_str, os_process_state_str(pcb->state));
            char resource_str[50];
            strcpy(resource_str, os_resource_str(pcb->blockedResource)); 
            gtk_list_store_set(blocked_queue_store, &iter,
                               0, pcb->id,
                               1, state_str,
                               2, resource_str, // Use API to convert resource to string
                               -1);
        }
        current_blocked = current_blocked->next;
    }
}

static void update_resource_management() {
    // This function might need to be re-evaluated based on how os_logic exposes semaphore/mutex states.
    // For now, let's assume os_logic handles resource contention internally and we display blocked processes.
    gtk_list_store_clear(blocked_resources_store);
    // The blocked_queue_view already shows processes and their blocked resources.
    // If more detailed mutex status is needed, os_logic.h would need to provide an API for it.
    add_event_message("Resource management view updated (primarily via blocked queue).");
}

static void update_memory_viewer() {
    Memory current_memory;
    os_get_memory(&current_memory);

    for (int i = 0; i < MEMORY_SIZE; ++i) {
        char cell_text[512]; // Increased buffer size
        if (strlen(current_memory.memoryArray[i].name) > 0) {
            // If name is present, it's likely an instruction or variable name
            snprintf(cell_text, sizeof(cell_text), "P%s: %s", 
                     current_memory.memoryArray[i].data, // Assuming data field stores Process ID or similar identifier
                     current_memory.memoryArray[i].name);
            gtk_widget_set_name(memory_cells[i], "memory-cell-used");
        } else if (strlen(current_memory.memoryArray[i].data) > 0) {
            // If only data is present (e.g. for variables without explicit names in this view)
            snprintf(cell_text, sizeof(cell_text), "Data: %s", current_memory.memoryArray[i].data);
            gtk_widget_set_name(memory_cells[i], "memory-cell-used");
        } else {
            snprintf(cell_text, sizeof(cell_text), "Free");
            gtk_widget_set_name(memory_cells[i], "memory-cell");
        }
        gtk_label_set_text(GTK_LABEL(memory_cells[i]), cell_text);
    }
    // Force re-application of CSS if needed, though set_name should trigger it.
    gtk_style_context_reset_widgets(gdk_screen_get_default());
}

static void add_execution_log(const char *message) {
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(execution_log_buffer, &iter);

    // Add timestamp
    char timestamp[20];
    sprintf(timestamp, "[Cycle %d] ", cycle_count-1);
    gtk_text_buffer_insert(execution_log_buffer, &iter, timestamp, -1);

    // Add message and new line
    gtk_text_buffer_insert(execution_log_buffer, &iter, message, -1);
    gtk_text_buffer_insert(execution_log_buffer, &iter, "\n", -1);

    // Scroll to the end
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(execution_log_view), &iter, 0.0, FALSE, 0.0, 0.0);
}

static void add_event_message(const char *message) {
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(event_messages_buffer, &iter);

    // Add timestamp
    char timestamp[20];
    sprintf(timestamp, "[Cycle %d] ", cycle_count);
    gtk_text_buffer_insert(event_messages_buffer, &iter, timestamp, -1);

    // Add message and new line
    gtk_text_buffer_insert(event_messages_buffer, &iter, message, -1);
    gtk_text_buffer_insert(event_messages_buffer, &iter, "\n", -1);

    // Scroll to the end
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(event_messages_view), &iter, 0.0, FALSE, 0.0, 0.0);
}

static void on_algorithm_changed(GtkComboBox *widget, gpointer user_data) {
    // If simulation is not running, we can allow changing the algorithm.
    // os_start will be called when the user presses start, with the new algorithm.
    if (!simulation_running) {
        const char *active_id = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
        if (active_id) {
            add_event_message(g_strconcat("Scheduler algorithm selected: ", active_id, NULL));
            if (strcmp(active_id, "Round Robin") == 0) {
                gtk_widget_set_sensitive(quantum_spin, TRUE);
            } else {
                gtk_widget_set_sensitive(quantum_spin, FALSE);
            }
            g_free((void*)active_id);
        }
    }
}

static void on_start_button_clicked(GtkButton *button, gpointer user_data) {
    if (simulation_running) {
        add_event_message("Simulation is already running.");
        return;
    }
    simulation_running = true;
    gtk_widget_set_sensitive(start_button, FALSE);
    gtk_widget_set_sensitive(stop_button, TRUE);
    gtk_widget_set_sensitive(reset_button, FALSE);
    gtk_widget_set_sensitive(step_button, FALSE);
    gtk_widget_set_sensitive(step5_button, FALSE);
    gtk_widget_set_sensitive(add_process_button, FALSE);
    gtk_widget_set_sensitive(algorithm_combo, FALSE);
    gtk_widget_set_sensitive(quantum_spin, FALSE);

    SchedulerType selected_type;
    const char *active_id = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(algorithm_combo));
    if (strcmp(active_id, "FCFS") == 0) selected_type = SCHED_FCFS;
    else if (strcmp(active_id, "Round Robin") == 0) selected_type = SCHED_ROUND_ROBIN;
    else if (strcmp(active_id, "MLFQ") == 0) selected_type = SCHED_MLFQ;
    else selected_type = SCHED_FCFS; // Default
    free((void*)active_id); // free the duplicated string

    int quantum = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(quantum_spin));
    os_start(selected_type, quantum);

    add_event_message("Simulation started.");
    if (main_loop == NULL) {
        main_loop = g_main_loop_new(NULL, FALSE);
    }
    timeout_id = g_timeout_add(1000, simulation_step, NULL); // 1 second interval
    // g_main_loop_run(main_loop); // This would block, run in a separate thread if needed or rely on timeout
}

static void on_stop_button_clicked(GtkButton *button, gpointer user_data) {
    if (!simulation_running) {
        add_event_message("Simulation is not running.");
        return;
    }
    simulation_running = false;
    if (timeout_id > 0) {
        g_source_remove(timeout_id);
        timeout_id = 0;
    }
    // if (main_loop != NULL && g_main_loop_is_running(main_loop)) {
    //     g_main_loop_quit(main_loop);
    // }

    gtk_widget_set_sensitive(start_button, TRUE);
    gtk_widget_set_sensitive(stop_button, FALSE);
    gtk_widget_set_sensitive(reset_button, TRUE);
    gtk_widget_set_sensitive(step_button, TRUE);
    gtk_widget_set_sensitive(step5_button, TRUE);
    gtk_widget_set_sensitive(add_process_button, TRUE);
    gtk_widget_set_sensitive(algorithm_combo, TRUE);
    gtk_widget_set_sensitive(quantum_spin, TRUE);
    add_event_message("Simulation stopped.");
}

static void on_reset_button_clicked(GtkButton *button, gpointer user_data) {
    os_reset();
    cycle_count = 0;
    // num_loaded_processes = 0; // Resetting this here might clear processes before os_reset handles them
                               // os_logic should manage its internal process list.

    // Clear GUI elements
    gtk_list_store_clear(process_list_store);
    gtk_list_store_clear(ready_queue_store);
    gtk_list_store_clear(blocked_queue_store);
    gtk_list_store_clear(running_process_store);
    gtk_text_buffer_set_text(execution_log_buffer, "", -1);
    gtk_text_buffer_set_text(event_messages_buffer, "", -1);

    // Reset UI state
    gtk_widget_set_sensitive(start_button, TRUE);
    gtk_widget_set_sensitive(stop_button, FALSE);
    gtk_widget_set_sensitive(reset_button, TRUE);
    gtk_widget_set_sensitive(step_button, TRUE);
    gtk_widget_set_sensitive(step5_button, TRUE);
    gtk_widget_set_sensitive(add_process_button, TRUE);
    gtk_widget_set_sensitive(algorithm_combo, TRUE);
    gtk_widget_set_sensitive(quantum_spin, TRUE);
    simulation_running = false;
    if (timeout_id > 0) {
        g_source_remove(timeout_id);
        timeout_id = 0;
    }
    // if (main_loop != NULL && g_main_loop_is_running(main_loop)) {
    //     g_main_loop_quit(main_loop);
    // }
    // if (main_loop != NULL) {
    //     g_main_loop_unref(main_loop);
    //     main_loop = NULL;
    // }

    update_dashboard();
    update_memory_viewer();
    add_event_message("Simulation reset.");
}

static void on_step_button_clicked(GtkButton *button, gpointer user_data) {
    if (simulation_running) {
        add_event_message("Cannot step while simulation is running automatically. Stop first.");
        return;
    }
    os_step();
    update_dashboard();
    update_memory_viewer();
    update_resource_management(); 
    add_event_message("Simulation stepped forward by one cycle.");
    if(os_is_finished()){
        add_event_message("All processes terminated. Simulation finished.");
        on_stop_button_clicked(NULL,NULL); // Reuse stop logic
    }
}

static void on_step5_clicked(GtkButton *button, gpointer user_data) {
    if (simulation_running) {
        add_event_message("Cannot step while simulation is running automatically. Stop first.");
        return;
    }
    for(int i=0; i<5; ++i){
        os_step();
        if(os_is_finished()){
            add_event_message("All processes terminated during 5-step. Simulation finished.");
            break; 
        }
    }
    update_dashboard();
    update_memory_viewer();
    update_resource_management();
    add_event_message("Simulation stepped forward by up to 5 cycles.");
    if(os_is_finished()){
        on_stop_button_clicked(NULL,NULL); // Reuse stop logic
    }
}

static void on_add_process_button_clicked(GtkButton *button, gpointer user_data) {
    // This function now iterates through the GUI entries and calls os_add_process
    // It assumes os_add_process will handle loading and placing into a loading queue if necessary.
    for (int i = 0; i < 10; ++i) {
        const char *filepath = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(process_files_combo[i]));
        if (filepath && strlen(filepath) > 0) {
            int arrival_time = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(arrival_time_entries[i]));
            ProcessControlBlock* new_pcb = os_add_process(filepath, arrival_time);
            if (new_pcb) {
                char msg[300];
                snprintf(msg, sizeof(msg), "Process %d from %s added with arrival time %d.", new_pcb->id, filepath, arrival_time);
                add_event_message(msg);
            } else {
                char msg[300];
                snprintf(msg, sizeof(msg), "Failed to add process from %s (max processes reached or file error).", filepath);
                add_event_message(msg);
            }
            g_free((void*)filepath); // Free the string obtained from get_active_text
        }
    }
    update_dashboard(); // Update display after adding processes
}

static gboolean simulation_step(gpointer user_data) {
    if (!simulation_running) {
        return G_SOURCE_REMOVE; // Stop the timer if simulation was stopped
    }

    os_step();
    update_dashboard();
    update_memory_viewer();
    update_resource_management();

    if (os_is_finished()) {
        add_event_message("All processes terminated. Simulation finished.");
        on_stop_button_clicked(NULL, NULL); // Reuse stop logic to update UI and state
        return G_SOURCE_REMOVE; // Stop the timer
    }

    return G_SOURCE_CONTINUE; // Continue the timer
}