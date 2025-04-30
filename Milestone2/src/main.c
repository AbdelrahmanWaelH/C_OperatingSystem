 #include <gtk/gtk.h>
 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>
 #include <stdbool.h>
 #include "../include/FileReader.h"
#include "../include/Queue.h"
#include <semaphore.h>
#include <stdarg.h>

#define CODE_SEGMENT_OFFSET 9
#define MLFQ_LEVELS 4
typedef enum { READY, RUNNING, BLOCKED,TERMINATED } ProcessState;

typedef enum {
	USER_INPUT,
	USER_OUTPUT,
	FILE_RW,
	NIL,
} Resource;

typedef struct {
	int id;
	ProcessState state;
	int priority;
	int memory_start;
	int memory_end;
	int program_counter;

	// virtual values
	Resource blockedResource;
	int timeslice_used;
    int arrivalTime;
	int time_in_queue;
} ProcessControlBlock; // 6 words


typedef enum { SCHED_FCFS, SCHED_ROUND_ROBIN, SCHED_MLFQ } SchedulerType;

typedef struct Scheduler {
	SchedulerType type;
	Queue* input_queues[MLFQ_LEVELS];
	Queue* output_queue;
	int mlfq_quantum[MLFQ_LEVELS];
	int rr_quantum;
    ProcessControlBlock* running_process;
} Scheduler;


typedef struct {

	char name[256];
	char data[256];

} MemoryWord;

typedef struct {
	MemoryWord memoryArray[60];
} Memory;

extern sem_t userInputSemaphore;
extern sem_t userOutputSemaphore;
extern sem_t fileSemaphore;

ProcessControlBlock* userInputBlockingProcess;
ProcessControlBlock* userOutputBlockingProcess;
ProcessControlBlock* fileBlockingProcess;

int processIdCounter = 0;

void removeNewline(char* buffer)
{
	// Find the newline character (if any) and replace it with '\0'
	size_t length = strlen(buffer);
	if (length > 0 && buffer[length - 1] == '\n') {
		buffer[length - 1] = '\0';
	}
}
Scheduler* scheduler_new(SchedulerType type, ...);
void scheduler_add_task(Scheduler* sched, ProcessControlBlock* process);
void scheduler_step(Scheduler* sched);
void scheduler_free(Scheduler* sched);

void initMemory();
void initPCBs();
void initSemaphores();
void initQueues();
char* processToString(ProcessState state);
void displayMemory(Memory* mainMemory);
char* select_file_dialog(GtkWidget *parent_window);

 // Forward declarations for existing scheduler functions
 extern void executeSingleLinePCB(ProcessControlBlock* processControlBlock);
 extern ProcessControlBlock loadProcess(char* filepath);
 extern Scheduler* scheduler_new(SchedulerType type, ...);
 extern void scheduler_add_task(Scheduler* sched, ProcessControlBlock* process);
 extern void scheduler_step(Scheduler* sched);
 extern void initMemory();
 extern void initSemaphores();
 extern void displayMemory(Memory* mainMemory);
 char* resourceToString(Resource resource);

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
 Scheduler *scheduler = NULL;
 GMainLoop *main_loop = NULL;
 guint timeout_id = 0;

 // List of loaded processes
 ProcessControlBlock *loaded_processes[10];
 int num_loaded_processes = 0;

 // Reference to global memory
 extern Memory mainMemory;
 extern Queue blockedQueue;
 extern sem_t userInputSemaphore, userOutputSemaphore, fileSemaphore;

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

    // Initialize the simulation
    initMemory();
    initSemaphores();

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
     GtkWidget *current_cycle_title = gtk_label_new("Current Cycle:");
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
    // Update total processes
    char total_str[10];
    sprintf(total_str, "%d", num_loaded_processes);
    gtk_label_set_text(GTK_LABEL(total_processes_label), total_str);

    // Update current cycle
    char cycle_str[10];
    sprintf(cycle_str, "%d", cycle_count);
    gtk_label_set_text(GTK_LABEL(current_cycle_label), cycle_str);

    // Update active algorithm
    int active_algo = gtk_combo_box_get_active(GTK_COMBO_BOX(algorithm_combo));
    const char *algo_name;

    switch (active_algo) {
        case 0:
            algo_name = "FCFS";
            break;
        case 1:
            algo_name = "Round Robin";
            break;
        case 2:
            algo_name = "MLFQ";
            break;
        default:
            algo_name = "Unknown";
    }

    gtk_label_set_text(GTK_LABEL(algorithm_label), algo_name);

    // Update process list, queues, and other displays
    update_process_list();
    update_queues();
    update_resource_management();
    update_memory_viewer();
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
    // Clear current list
    gtk_list_store_clear(process_list_store);

    // Add all loaded processes
    for (int i = 0; i < num_loaded_processes; i++) {
        ProcessControlBlock *pcb = loaded_processes[i];
        if (pcb == NULL) continue;

        GtkTreeIter iter;
        gtk_list_store_append(process_list_store, &iter);

        // Determine state string
        const char *state = processToString(pcb->state);

        // Add process to the list
        gtk_list_store_set(process_list_store, &iter,
            0, pcb->id,
            1, state,
            2, pcb->priority,
            3, pcb->memory_start,  // Fixed field name
            4, pcb->memory_end,    // Fixed field name
            5, pcb->program_counter,
            -1);
    }
}

static void update_queues() {
    // Clear current queues
    gtk_list_store_clear(ready_queue_store);
    gtk_list_store_clear(blocked_queue_store);
    gtk_list_store_clear(running_process_store);

    if (scheduler == NULL) return;

    // Update running process display
    if (scheduler->running_process != NULL) {
        ProcessControlBlock *process = scheduler->running_process;
        GtkTreeIter iter;
        gtk_list_store_append(running_process_store, &iter);

        // Get current instruction
        char instruction[256] = "Unknown";
        if (process->program_counter >= 0 &&
            process->memory_start + process->program_counter + CODE_SEGMENT_OFFSET < process->memory_end) {
            // Get the instruction from memory or program
            // This depends on how you store instructions in your implementation
            // Either fetch from memory or from process->instructions if available
            strcpy(instruction, "Current instruction"); // Replace with actual instruction
        }

        gtk_list_store_set(running_process_store, &iter,
                          0, process->id,
                          1, instruction,
                          2, process->timeslice_used, // Time running
                          -1);
    }

    // Update ready queues based on scheduler type
    for (int level = 0; level < (scheduler->type == SCHED_MLFQ ? MLFQ_LEVELS : 1); level++) {
        Queue* current_queue = scheduler->input_queues[level];
        if (current_queue == NULL || queue_is_empty(current_queue)) continue;

        // Iterate through the queue without modifying it
        Node* current = current_queue->head;
        int position = 0;

        while (current != NULL) {
            ProcessControlBlock* process = (ProcessControlBlock*)current->data;
            GtkTreeIter iter;
            gtk_list_store_append(ready_queue_store, &iter);

            // Get current instruction
            char instruction[256] = "Unknown";
            if (process->program_counter >= 0 &&
                process->memory_start + process->program_counter + CODE_SEGMENT_OFFSET < process->memory_end) {
                // Get the instruction
            	int memoryStart = process->memory_start;
            	int currentLine = memoryStart + process->program_counter + CODE_SEGMENT_OFFSET;
            	char* line = mainMemory.memoryArray[currentLine].data;
            	if (line != NULL) {
            		snprintf(instruction, sizeof(instruction), "%s", line);
            	}
            }

            // For MLFQ, show which level the process is in
            char queue_info[32];
            if (scheduler->type == SCHED_MLFQ) {
                sprintf(queue_info, "Queue %d", level);
            } else {
                sprintf(queue_info, "Position %d", position);
            }

            gtk_list_store_set(ready_queue_store, &iter,
                              0, process->id,
                              1, instruction,
                              2, process->time_in_queue,
                              -1);

            current = current->next;
            position++;
        }
    }

    // Update blocked queue
    if (!queue_is_empty(&blockedQueue)) {
        Node* current = blockedQueue.head;

        while (current != NULL) {
            ProcessControlBlock* process = (ProcessControlBlock*)current->data;
            GtkTreeIter iter;
            gtk_list_store_append(blocked_queue_store, &iter);

            // Determine blocking resource
            char resource[32] = "Unknown";
            switch (process->blockedResource) {
                case FILE_RW:
                    strcpy(resource, "File I/O");
                    break;
                case USER_INPUT:
                    strcpy(resource, "User Input");
                    break;
                case USER_OUTPUT:
                    strcpy(resource, "User Output");
                    break;
                default:
                    strcpy(resource, "Unknown");
                    break;
            }

            gtk_list_store_set(blocked_queue_store, &iter,
                              0, process->id,
                              1, resource,
                              2, process->timeslice_used, // Time blocked
                              -1);

            current = current->next;
        }
    }
}

static void update_resource_management() {
    // Clear current blocked resources list
    gtk_list_store_clear(blocked_resources_store);

    // Update semaphore statuses
    int userInputValue, userOutputValue, fileValue;
    sem_getvalue(&userInputSemaphore, &userInputValue);
    sem_getvalue(&userOutputSemaphore, &userOutputValue);
    sem_getvalue(&fileSemaphore, &fileValue);

    // Get the labels in the grid
    GtkWidget *user_input_status = gtk_grid_get_child_at(GTK_GRID(mutex_status_grid), 1, 1);
    GtkWidget *user_input_holder = gtk_grid_get_child_at(GTK_GRID(mutex_status_grid), 2, 1);
    GtkWidget *user_output_status = gtk_grid_get_child_at(GTK_GRID(mutex_status_grid), 1, 2);
    GtkWidget *user_output_holder = gtk_grid_get_child_at(GTK_GRID(mutex_status_grid), 2, 2);
    GtkWidget *file_status = gtk_grid_get_child_at(GTK_GRID(mutex_status_grid), 1, 3);
    GtkWidget *file_holder = gtk_grid_get_child_at(GTK_GRID(mutex_status_grid), 2, 3);

    // Update status based on semaphore values
    gtk_label_set_text(GTK_LABEL(user_input_status), userInputValue > 0 ? "Free" : "Locked");
    gtk_label_set_text(GTK_LABEL(user_output_status), userOutputValue > 0 ? "Free" : "Locked");
    gtk_label_set_text(GTK_LABEL(file_status), fileValue > 0 ? "Free" : "Locked");

	if (userInputValue > 0) {
		userInputBlockingProcess = NULL;
	}

	if (userOutputValue > 0) {
		userOutputBlockingProcess = NULL;
	}

	if (fileValue > 0) {
		fileBlockingProcess = NULL;
	}

	char buffer[100];

	// For user input
	if (userInputBlockingProcess == NULL) {
		gtk_label_set_text(GTK_LABEL(user_input_holder), "None");
	} else {
		snprintf(buffer, sizeof(buffer), "Process %d", userInputBlockingProcess->id);
		gtk_label_set_text(GTK_LABEL(user_input_holder), buffer);
	}

	// For user output
	if (userOutputBlockingProcess == NULL) {
		gtk_label_set_text(GTK_LABEL(user_output_holder), "None");
	} else {
		snprintf(buffer, sizeof(buffer), "Process %d", userOutputBlockingProcess->id);
		gtk_label_set_text(GTK_LABEL(user_output_holder), buffer);
	}

	// For file
	if (fileBlockingProcess == NULL) {
		gtk_label_set_text(GTK_LABEL(file_holder), "None");
	} else {
		snprintf(buffer, sizeof(buffer), "Process %d", fileBlockingProcess->id);
		gtk_label_set_text(GTK_LABEL(file_holder), buffer);
	}


    // Populate blocked resources list
    // This would depend on how your system tracks blocked processes
    // Here's a simplified example
    Node *current = blockedQueue.head;

    while (current != NULL) {
        ProcessControlBlock *pcb = (ProcessControlBlock *)current->data;
        if (pcb->blockedResource != NIL) {
            GtkTreeIter iter;
            gtk_list_store_append(blocked_resources_store, &iter);

            gtk_list_store_set(blocked_resources_store, &iter,
                              0, pcb->id,
                              1, resourceToString(pcb->blockedResource),
                              2, pcb->priority,
                              -1);
        }
        current = current->next;
    }
}

static void update_memory_viewer() {
    // This function should update the memory visualization
    // For each memory cell, update its content
    for (int i = 0; i < 60; i++) { // Use hardcoded 60 or define a MEMORY_SIZE constant
        // Get the name and data widgets
        GtkWidget *box = memory_cells[i];
        if (box == NULL) {
            continue; // Skip if this memory cell widget doesn't exist
        }

        // Get the list of children ONCE
        GList *children = gtk_container_get_children(GTK_CONTAINER(box));
        if (children == NULL || children->data == NULL) {
            continue; // Skip if no children or first child is NULL
        }

        GtkWidget *name_label = GTK_WIDGET(children->data);

        // Check if there's a second child before accessing it
        GtkWidget *data_label = NULL;
        if (children->next != NULL && children->next->data != NULL) {
            data_label = GTK_WIDGET(children->next->data);
        } else {
            // Free the list and skip this iteration if there's no valid second child
            g_list_free(children);
            continue;
        }

        // Set the process name that owns this memory cell
        char name_str[20] = "";
        for (int j = 0; j < num_loaded_processes; j++) {
            ProcessControlBlock *pcb = loaded_processes[j];
            if (pcb != NULL &&
                i >= pcb->memory_start &&
                i <= pcb->memory_end) {
                sprintf(name_str, "P%d", pcb->id);
                break;
            }
        }

        gtk_label_set_text(GTK_LABEL(name_label), name_str);

        // Set the data content - ensure valid data is available
        if (i < sizeof(mainMemory.memoryArray) / sizeof(mainMemory.memoryArray[0]) &&
            mainMemory.memoryArray[i].data != NULL &&
            strlen(mainMemory.memoryArray[i].data) > 0) {
            gtk_label_set_text(GTK_LABEL(data_label), mainMemory.memoryArray[i].data);
        } else {
            gtk_label_set_text(GTK_LABEL(data_label), "");
        }

        // Add highlighting if the cell is used
        if (strlen(name_str) > 0) {
            GtkStyleContext *context = gtk_widget_get_style_context(name_label);
            gtk_style_context_add_class(context, "memory-cell-used");

            if (data_label != NULL) {
                context = gtk_widget_get_style_context(data_label);
                gtk_style_context_add_class(context, "memory-cell-used");
            }
        } else {
            GtkStyleContext *context = gtk_widget_get_style_context(name_label);
            gtk_style_context_remove_class(context, "memory-cell-used");

            if (data_label != NULL) {
                context = gtk_widget_get_style_context(data_label);
                gtk_style_context_remove_class(context, "memory-cell-used");
            }
        }

        // Free the list when done with it
        g_list_free(children);
    }
}

static void add_execution_log(const char *message) {
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(execution_log_buffer, &iter);

    // Add timestamp
    char timestamp[20];
    sprintf(timestamp, "[Cycle %d] ", cycle_count);
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
    // Get the selected algorithm
    int selected = gtk_combo_box_get_active(widget);

    // Enable/disable quantum spin button based on selection
    if (selected == 1) {  // Round Robin
        gtk_widget_set_sensitive(quantum_spin, TRUE);
    } else {
        gtk_widget_set_sensitive(quantum_spin, FALSE);
    }

    // Update algorithm label
    const char *algo_name;
    switch (selected) {
        case 0:
            algo_name = "FCFS";
            break;
        case 1:
            algo_name = "Round Robin";
            break;
        case 2:
            algo_name = "MLFQ";
            break;
        default:
            algo_name = "Unknown";
    }
    gtk_label_set_text(GTK_LABEL(algorithm_label), algo_name);

    // If the simulation is already running, we should recreate the scheduler
    if (simulation_running) {
        on_stop_button_clicked(NULL, NULL);
        add_event_message("Scheduling algorithm changed, simulation stopped.");
    }
}

static void on_start_button_clicked(GtkButton *button, gpointer user_data) {
    if (simulation_running) return;

    // Check if we have processes loaded
    if (num_loaded_processes == 0) {
        add_event_message("Cannot start simulation: No processes loaded.");
        return;
    }

    // Determine scheduler type
    SchedulerType type;
    int selected = gtk_combo_box_get_active(GTK_COMBO_BOX(algorithm_combo));

    switch (selected) {
        case 0:
            type = SCHED_FCFS;
            break;
        case 1:
            type = SCHED_ROUND_ROBIN;
            break;
        case 2:
            type = SCHED_MLFQ;
            break;
        default:
            type = SCHED_FCFS;
    }

    // Create scheduler with appropriate parameters
    if (selected == 1) {  // Round Robin with quantum
        int quantum = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(quantum_spin));
        scheduler = scheduler_new(type, quantum);
    } else {
        scheduler = scheduler_new(type);
    }

    // Add all loaded processes to the scheduler
    for (int i = 0; i < num_loaded_processes; i++) {
        if (loaded_processes[i] != NULL) {
            scheduler_add_task(scheduler, loaded_processes[i]);
        }
    }

    // Start the simulation loop
    simulation_running = true;
    timeout_id = g_timeout_add(200, simulation_step, NULL); // Run every 200ms

    // Update button states
    gtk_widget_set_sensitive(start_button, FALSE);
    gtk_widget_set_sensitive(stop_button, TRUE);
    gtk_widget_set_sensitive(step_button, FALSE);
	gtk_widget_set_sensitive(step5_button, FALSE);
    gtk_widget_set_sensitive(add_process_button, FALSE);
    gtk_widget_set_sensitive(algorithm_combo, FALSE);
    gtk_widget_set_sensitive(quantum_spin, FALSE);

    add_event_message("Simulation started.");
}

static void on_stop_button_clicked(GtkButton *button, gpointer user_data) {
    if (!simulation_running) return;

    // Stop the simulation loop
    if (timeout_id > 0) {
        g_source_remove(timeout_id);
        timeout_id = 0;
    }

    simulation_running = false;

    // Update button states
    gtk_widget_set_sensitive(start_button, TRUE);
    gtk_widget_set_sensitive(stop_button, FALSE);
    gtk_widget_set_sensitive(step5_button, TRUE);
    gtk_widget_set_sensitive(add_process_button, TRUE);
    gtk_widget_set_sensitive(algorithm_combo, TRUE);

    // Enable quantum spin if Round Robin is selected
    if (gtk_combo_box_get_active(GTK_COMBO_BOX(algorithm_combo)) == 1) {
        gtk_widget_set_sensitive(quantum_spin, TRUE);
    }

    add_event_message("Simulation stopped.");
}

static void on_reset_button_clicked(GtkButton *button, gpointer user_data) {
    // Stop the simulation if it's running
    if (simulation_running) {
        on_stop_button_clicked(NULL, NULL);
    }

    // Clear all processes
    for (int i = 0; i < num_loaded_processes; i++) {
        if (loaded_processes[i] != NULL) {
            // Free process memory (if needed)
            loaded_processes[i] = NULL;
        }
    }
	// reset number of loaded processes
    num_loaded_processes = 0;

    // Reset cycle count
    cycle_count = 0;

	// reset the processIdCounter
	processIdCounter = 0;

	// clear the scheduler
	scheduler = NULL;

    // Reinitialize memory and semaphores
    initMemory();
    initSemaphores();
	// reset the sempahore locked processes
	fileBlockingProcess = NULL;
	userOutputBlockingProcess = NULL;
	userInputBlockingProcess = NULL;

    // Clear all displays
    gtk_list_store_clear(process_list_store);
    gtk_list_store_clear(ready_queue_store);
    gtk_list_store_clear(blocked_queue_store);
    gtk_list_store_clear(running_process_store);
    gtk_list_store_clear(blocked_resources_store);
	for (int i = 0; i < 60 ; i++) {
		memset(mainMemory.memoryArray[i].data, 0, sizeof(mainMemory.memoryArray[i].data));
		memset(mainMemory.memoryArray[i].name, 0, sizeof(mainMemory.memoryArray[i].name));
	}
    // Clear text buffers
    gtk_text_buffer_set_text(execution_log_buffer, "", -1);
    gtk_text_buffer_set_text(event_messages_buffer, "", -1);

    // Update displays
    update_dashboard();

    add_event_message("Simulation reset. All processes and memory cleared.");
}

static void on_step_button_clicked(GtkButton *button, gpointer user_data) {
    if (simulation_running) return;

    // Check if we have processes loaded
    if (num_loaded_processes == 0) {
        add_event_message("Cannot step simulation: No processes loaded.");
        return;
    }

    // Create scheduler if not exists
    if (scheduler == NULL) {
        // Determine scheduler type
        SchedulerType type;
        int selected = gtk_combo_box_get_active(GTK_COMBO_BOX(algorithm_combo));

        switch (selected) {
            case 0:
                type = SCHED_FCFS;
                break;
            case 1:
                type = SCHED_ROUND_ROBIN;
                break;
            case 2:
                type = SCHED_MLFQ;
                break;
            default:
                type = SCHED_FCFS;
        }

        // Create scheduler with appropriate parameters
        if (selected == 1) {  // Round Robin with quantum
            int quantum = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(quantum_spin));
            scheduler = scheduler_new(type, quantum);
        } else {
            scheduler = scheduler_new(type);
        }

        // Add all loaded processes to the scheduler
        for (int i = 0; i < num_loaded_processes; i++) {
            if (loaded_processes[i] != NULL) {
                scheduler_add_task(scheduler, loaded_processes[i]);
            }
        }

        add_event_message("Scheduler initialized.");
    }

    // Execute one step
    simulation_step(NULL);
}

static void on_step5_clicked(GtkButton *button, gpointer user_data) {
	for (int i=0;i<5;i++) {
		on_step_button_clicked(button,user_data);
	}
}
// Function to create a file chooser dialog and return the selected filename
char* select_file_dialog(GtkWidget *parent_window) {
    GtkWidget *dialog;
    char *filename = NULL;

    dialog = gtk_file_chooser_dialog_new("Select Process File",
                                        GTK_WINDOW(parent_window),
                                        GTK_FILE_CHOOSER_ACTION_OPEN,
                                        "Cancel", GTK_RESPONSE_CANCEL,
                                        "Open", GTK_RESPONSE_ACCEPT,
                                        NULL);

    // Set up a filter for .txt files
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Text Files");
    gtk_file_filter_add_pattern(filter, "*.txt");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    // Show the dialog and wait for a response
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    }

    gtk_widget_destroy(dialog);
    return filename;
}

static void on_add_process_button_clicked(GtkButton *button, gpointer user_data) {
    // Count how many processes were selected
    int count = 0;
    char *selected_files[10];
    int arrival_times[10];

    for (int i = 0; i < 10; i++) {
        const char *file = gtk_entry_get_text(GTK_ENTRY(gtk_bin_get_child(GTK_BIN(process_files_combo[i]))));
        if (file != NULL && strlen(file) > 0) {
            selected_files[count] = strdup(file);
            arrival_times[count] = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(arrival_time_entries[i]));
            count++;
        }
    }

    if (count == 0) {
        add_event_message("No processes selected.");
        return;
    }

    // Load each selected process
    for (int i = 0; i < count; i++) {
        // Check if we have space for more processes
        if (num_loaded_processes >= 10) {
            add_event_message("Cannot load more processes: Maximum limit reached.");
            free(selected_files[i]);
            continue;
        }

        // Load the process
        ProcessControlBlock pcb = loadProcess(selected_files[i]);
        // Set arrival time - only if you've added the field to the struct
        pcb.arrivalTime = arrival_times[i];

        // Store the process
        loaded_processes[num_loaded_processes] = malloc(sizeof(ProcessControlBlock));
        memcpy(loaded_processes[num_loaded_processes], &pcb, sizeof(ProcessControlBlock));
        num_loaded_processes++;

        // Log the event
        char message[256];
        sprintf(message, "Process loaded from %s with arrival time %d.",
                selected_files[i], arrival_times[i]);
        add_event_message(message);

        // Free the file name
        free(selected_files[i]);
    }

    // Update the displays
    update_dashboard();

	// redirect to dashboard window using the global notebook
	gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 0);
}

static gboolean simulation_step(gpointer user_data) {
    // Execute one simulation step
    cycle_count++;

    // Update the cycle count label
    char cycle_str[10];
    sprintf(cycle_str, "%d", cycle_count);
    gtk_label_set_text(GTK_LABEL(current_cycle_label), cycle_str);

    // Step the scheduler
    if (scheduler != NULL) {
        char log_message[256];

        // Check if we have a running process
        if (scheduler->running_process != NULL) {
            ProcessControlBlock *running = scheduler->running_process;

        	char instruction[256] = "Unknown";
        	if (running->state != TERMINATED) {
        		// Get the instruction
        		int memoryStart = running->memory_start;
        		int currentLine = memoryStart + running->program_counter + CODE_SEGMENT_OFFSET;
        		char* line = mainMemory.memoryArray[currentLine].data;
        		if (line != NULL) {
        			snprintf(instruction, sizeof(instruction), "%s", line);
        		}
				}

            // Log the current instruction - access it from memory
            int currentLine = running->memory_start + running->program_counter + CODE_SEGMENT_OFFSET;

            if (running->program_counter >= 0 &&
                currentLine <= running->memory_end) {
                sprintf(log_message, "Process %d executing: %s",running->id, instruction);
                add_execution_log(log_message);
            }
        }

        // Step the scheduler
        scheduler_step(scheduler);

        // Check for finished processes
        bool all_terminated = true;
        for (int i = 0; i < num_loaded_processes; i++) {
            if (loaded_processes[i] != NULL && loaded_processes[i]->state != TERMINATED) {
                all_terminated = false;
                break;
            }
        }

        if (all_terminated) {
            add_event_message("All processes have terminated. Simulation complete.");
            on_stop_button_clicked(NULL, NULL);
            return FALSE; // Stop the timer
        }
    }

    // Update all displays
    update_dashboard();
    return TRUE; // Continue the timer
}
// --------------------------- main functionallity --------------------------

/*Safer posting for semaphore to avoid overflow/underflow*/
void safe_sem_post(sem_t* __sem);

/*Safer wait for semaphore to avoid overflow/underflow*/
void safe_sem_wait(sem_t* __sem);

ProcessControlBlock loadProcess(char* filepath);
void executeSingleLinePCB(ProcessControlBlock* processControlBlock);
char** lineParser(char* line);


Memory mainMemory;
Queue blockedQueue;

void executeSingleLinePCB(ProcessControlBlock* processControlBlock)
{
 //This is so i can commit :D
	int memoryStart = processControlBlock->memory_start;
	int currentLine = memoryStart + processControlBlock->program_counter + CODE_SEGMENT_OFFSET;

	// printf("%d\n", processControlBlock->id);

	char* line = mainMemory.memoryArray[currentLine].data;
	char** tokens = lineParser(line);

	// printf("Currently at PC: %d\n", processControlBlock->program_counter);

	/*
		Tokens:
			assign
			semWait
			semSignal
			print
			printFromTo
	*/

	if (strcmp(tokens[0], "assign") == 0) {
		char* target = tokens[1];

        bool flag = false;        
        for( int i =6; i<= 8; i++){
            if(strcmp(target, mainMemory.memoryArray[processControlBlock->memory_start + i].name) ==0){
                break;
            }
			printf("%d",strlen(mainMemory.memoryArray[processControlBlock->memory_start+i].name));
            if(strlen(mainMemory.memoryArray[processControlBlock->memory_start+i].data)==0){
                strcpy(mainMemory.memoryArray[processControlBlock->memory_start+i].name, target);
            	break;
            }
        }


		char* value = tokens[2];

		value[strcspn(value, "\r\n")] = '\0';

		if (strcmp(value, "readFile") == 0) {
			/*
			Handle Case for
				assign a readfile b
			*/

			char* filepathVar = tokens[3];
			// removeNewline(filepathVar);
			filepathVar[strcspn(filepathVar, "\r\n")] = '\0';

			char filepath[256];

			if (strcmp(filepathVar, "a") == 0) {
				// mainMemory.memoryArray[processControlBlock->memory_start + 6].data =
				// fileString;
				strcpy(
					filepath, mainMemory.memoryArray[processControlBlock->memory_start + 6].data);
			} else if (strcmp(filepathVar, "b") == 0) {
				// mainMemory.memoryArray[processControlBlock->memory_start + 7].data =
				// fileString;
				strcpy(
					filepath, mainMemory.memoryArray[processControlBlock->memory_start + 7].data);
			} else {
				// mainMemory.memoryArray[processControlBlock->memory_start + 8].data =
				// fileString;
				strcpy(
					filepath, mainMemory.memoryArray[processControlBlock->memory_start + 8].data);
			}


			char* fileString = readFile(filepath);

			// printf("Read File String: %s", fileString);

			// printf("Found file path\n");

			if (strcmp(target, mainMemory.memoryArray[processControlBlock->memory_start + 6].name) == 0) {
				// mainMemory.memoryArray[processControlBlock->memory_start + 6].data =
				// fileString;
				strcpy(
					mainMemory.memoryArray[processControlBlock->memory_start + 6].data, fileString);
			} else if (strcmp(target, mainMemory.memoryArray[processControlBlock->memory_start + 7].name) == 0) {
				// mainMemory.memoryArray[processControlBlock->memory_start + 7].data =
				// fileString;
				strcpy(
					mainMemory.memoryArray[processControlBlock->memory_start + 7].data, fileString);
			} else if (strcmp(target, mainMemory.memoryArray[processControlBlock->memory_start + 8].name) == 0) {
				// mainMemory.memoryArray[processControlBlock->memory_start + 8].data =
				// fileString;
				strcpy(
					mainMemory.memoryArray[processControlBlock->memory_start + 8].data, fileString);
			}

		} else if (strcmp(value, "input") == 0) {

			/*
			Handle Case for
				assign a input
			*/

            char userInputValue[256] = { 0 };
            char* user_input = get_user_input_from_dialog(processControlBlock->id);
            strcpy(userInputValue, user_input);

			if (strcmp(target, mainMemory.memoryArray[processControlBlock->memory_start + 6].name) == 0) {
				// mainMemory.memoryArray[processControlBlock->memory_start + 6].data =
				// userInputValue;
				strcpy(mainMemory.memoryArray[processControlBlock->memory_start + 6].data,
					userInputValue);

			} else if (strcmp(target, mainMemory.memoryArray[processControlBlock->memory_start + 7].name) == 0) {
				// mainMemory.memoryArray[processControlBlock->memory_start + 7].data =
				// userInputValue;
				strcpy(mainMemory.memoryArray[processControlBlock->memory_start + 7].data,
					userInputValue);
			} else if (strcmp(target, mainMemory.memoryArray[processControlBlock->memory_start + 8].name) == 0){
				// mainMemory.memoryArray[processControlBlock->memory_start + 8].data =
				// userInputValue;
				strcpy(mainMemory.memoryArray[processControlBlock->memory_start + 8].data,
					userInputValue);
			}

		} else {
			/*
	  Handle Case for
	  assign a b
	  */

			int indexOfTarget = 0;
			int indexOfValue = 0;
			for (int i = 6; i <= 8; i++) {
				if (strcmp(
						mainMemory.memoryArray[processControlBlock->memory_start + i].name, target)
					== 0) {
					indexOfTarget = i;
				}
				if (strcmp(
						mainMemory.memoryArray[processControlBlock->memory_start + i].name, value)
					== 0) {
					indexOfValue = i;
				}
			}

			// mainMemory.memoryArray[processControlBlock->memory_start +
			// indexOfTarget].data =
			// mainMemory.memoryArray[processControlBlock->memory_start +
			// indexOfValue].data;
			strcpy(mainMemory.memoryArray[processControlBlock->memory_start + indexOfTarget].data,
				mainMemory.memoryArray[processControlBlock->memory_start + indexOfValue].data);
		}

	} else if (strcmp(tokens[0], "print") == 0) {
		char* value = tokens[1];
		char data[256];

		if (strcmp(value, mainMemory.memoryArray[processControlBlock->memory_start + 6].name) == 0) {
			// mainMemory.memoryArray[processControlBlock->memory_start + 6].data =
			// userInputValue;
			strcpy(data, mainMemory.memoryArray[processControlBlock->memory_start + 6].data);

		} else if (strcmp(value, mainMemory.memoryArray[processControlBlock->memory_start + 7].name) == 0) {
			// mainMemory.memoryArray[processControlBlock->memory_start + 7].data =
			// userInputValue;
			strcpy(data, mainMemory.memoryArray[processControlBlock->memory_start + 7].data);
		} else if (strcmp(value, mainMemory.memoryArray[processControlBlock->memory_start + 8].name) == 0) {
			// mainMemory.memoryArray[processControlBlock->memory_start + 8].data =
			// userInputValue;
			strcpy(data, mainMemory.memoryArray[processControlBlock->memory_start + 8].data);
		}

		show_text_dialog(data);
	} else if (strcmp(tokens[0], "printFromTo") == 0) {
		char* start = tokens[1];
		char* end = tokens[2];

		end[strcspn(end, "\r\n")] = '\0';

		int indexOfValue = 0;
		char* dataStart;
		char* dataEnd;

		// displayMemory(&mainMemory);

		for (int i = 6; i <= 8; i++) {
			if (strcmp(mainMemory.memoryArray[processControlBlock->memory_start + i].name, start)
				== 0) {
				dataStart = mainMemory.memoryArray[processControlBlock->memory_start + i].data;
			}
		}
		for (int i = 6; i <= 8; i++) {

			if (strcmp(mainMemory.memoryArray[processControlBlock->memory_start + i].name, end)
				== 0) {
				dataEnd = mainMemory.memoryArray[processControlBlock->memory_start + i].data;
			}
		}

		int startInt = atoi(dataStart);
		int endInt = atoi(dataEnd);

        char buffer[4096];
        sprintf(buffer, "Numbers from %d to %d: \n", startInt, endInt);
        for (int i = startInt; i <= endInt; i++) {
            char num[16];
            sprintf(num, " %d", i);
            strncat(buffer, num, sizeof(buffer) - strlen(buffer) - 1);
        }

        show_text_dialog(buffer);
	} else if (strcmp(tokens[0], "semWait") == 0) {
		char* resource = tokens[1];
		int semValue;
		int* pid = malloc(sizeof(int));
		*pid = processControlBlock->id;
		if (strcmp(resource, "file\r\n") == 0 || strcmp(resource, "file") == 0) {
			sem_getvalue(&fileSemaphore, &semValue);
			if (semValue == 0) {
				processControlBlock->state = BLOCKED;
				processControlBlock->blockedResource = FILE_RW;
				return;
			} else {
				fileBlockingProcess = processControlBlock;
				safe_sem_wait(&fileSemaphore);
			}

		} else if (strcmp(resource, "userInput\r\n") == 0 || strcmp(resource, "userInput") == 0) {
			sem_getvalue(&userInputSemaphore, &semValue);
			if (semValue == 0) {
				processControlBlock->state = BLOCKED;
				processControlBlock->blockedResource = USER_INPUT;
				return;
			} else {
				userInputBlockingProcess = processControlBlock;
				safe_sem_wait(&userInputSemaphore);
			}
		} else if (strcmp(resource, "userOutput\r\n") == 0 || strcmp(resource, "userOutput") == 0) {
			sem_getvalue(&userOutputSemaphore, &semValue);
			if (semValue == 0) {
				processControlBlock->state = BLOCKED;
				processControlBlock->blockedResource = USER_OUTPUT;
				return;
			} else {
				userOutputBlockingProcess = processControlBlock;
				safe_sem_wait(&userOutputSemaphore);
			}
		}

	} else if (strcmp(tokens[0], "semSignal") == 0) {
		char* resource = tokens[1];
		if (strcmp(resource, "file\r\n") == 0 || strcmp(resource, "file") == 0) {
			safe_sem_post(&fileSemaphore);
		} else if (strcmp(resource, "userInput\r\n") == 0 || strcmp(resource, "userInput") == 0) {
			safe_sem_post(&userInputSemaphore);
		} else if (strcmp(resource, "userOutput\r\n") == 0 || strcmp(resource, "userOutput") == 0) {
			safe_sem_post(&userOutputSemaphore);
		}

	} else if (strcmp(tokens[0], "writeFile") == 0) {
		char* filenameVar = tokens[1];
		char* dataVar = tokens[2];
		dataVar[strcspn(dataVar, "\r\n")] = '\0';

		char* dataToWrite;
		char* filename;

		for (int i = 6; i <= 8; i++) {
			if (strcmp(mainMemory.memoryArray[processControlBlock->memory_start + i].name, dataVar)
				== 0) {
				dataToWrite = mainMemory.memoryArray[processControlBlock->memory_start + i].data;
			}
			if (strcmp(
					mainMemory.memoryArray[processControlBlock->memory_start + i].name, filenameVar)
				== 0) {
				filename = mainMemory.memoryArray[processControlBlock->memory_start + i].data;
			}
		}

		FILE* newFile = fopen(filename, "w");
		fprintf(newFile, "%s", dataToWrite);
		fflush(newFile);

	}

	else {
		// Unknown instruction
		printf("Unknown instruction: %s\n", tokens[0]);
	}

	processControlBlock->program_counter += 1;
}

char** lineParser(char* line)
{

	char** tokenArray
		= (char**)malloc(sizeof(char*) * 16); // Assuming maximum of 8 tokens per line.

	char lineCopy[strlen(line) + 10];
	line[strcspn(line, "\r\n")] = '\0';
	strcpy(lineCopy, line);
	char* token = strtok(lineCopy, " ");

	int counter = 0;

	while (token != NULL) {
		tokenArray[counter++] = strdup(token);
		token = strtok(NULL, " ");
	}
	tokenArray[counter] = NULL;

	return tokenArray;
}

void safe_sem_post(sem_t* __sem)
{

	int value;
	sem_getvalue(__sem, &value);

	if (value >= 1)
		return;

	sem_post(__sem);
}

void safe_sem_wait(sem_t* __sem)
{

	int value;
	sem_getvalue(__sem, &value);

	if (value <= 0)
		return;

	sem_wait(__sem);
}

void initMemory()
{
	for (int i = 0; i < 60; i++) {

		// mainMemory.memoryArray[i].name = malloc(64 * sizeof(char)); // or any
		// size you need mainMemory.memoryArray[i].data = malloc(64 * sizeof(char));
		// // or any size you need
	}
}

ProcessControlBlock loadProcess(char* filepath)
{

	ProcessControlBlock pcb;

	int indexOfFree = 0;
	while (strcmp(mainMemory.memoryArray[indexOfFree].name, "")) {
		indexOfFree++;
	}

	if (indexOfFree > 40) {
		perror("Not enough memory to store process in memory");
		return pcb;
	}

	pcb.id = processIdCounter;
	processIdCounter++;

	pcb.memory_start = indexOfFree;

	pcb.priority = 1;

	pcb.program_counter = 0;

	pcb.state = READY;
	pcb.blockedResource = NIL;
	pcb.timeslice_used = 0;
	pcb.time_in_queue = 0;
	//TODO the whole arrival time logic needs to be enhanced !
	pcb.arrivalTime = 0;
	char* buffer = malloc(256);

	// mainMemory.memoryArray[pcb.memory_start + 0].name = "pid";
	strcpy(mainMemory.memoryArray[pcb.memory_start + 0].name, "pid");
	sprintf(buffer, "%d", pcb.id);
	strcpy(mainMemory.memoryArray[pcb.memory_start + 0].data, buffer);

	// mainMemory.memoryArray[pcb.memory_start + 1].name = "mem_start";
	strcpy(mainMemory.memoryArray[pcb.memory_start + 1].name, "mem_start");
	sprintf(buffer, "%d", pcb.memory_start);
	strcpy(mainMemory.memoryArray[pcb.memory_start + 1].data, buffer);

	// mainMemory.memoryArray[pcb.memory_start + 2].name = "mem_end";
	strcpy(mainMemory.memoryArray[pcb.memory_start + 2].name, "mem_end");
	sprintf(buffer, "%d", pcb.memory_end);
	strcpy(mainMemory.memoryArray[pcb.memory_start + 2].data, buffer);

	// mainMemory.memoryArray[pcb.memory_start + 3].name = "priority";
	strcpy(mainMemory.memoryArray[pcb.memory_start + 3].name, "priority");
	sprintf(buffer, "%d", pcb.priority);
	strcpy(mainMemory.memoryArray[pcb.memory_start + 3].data, buffer);

	// mainMemory.memoryArray[pcb.memory_start + 4].name = "program_counter";
	strcpy(mainMemory.memoryArray[pcb.memory_start + 4].name, "program_counter");
	sprintf(buffer, "%d", pcb.program_counter);
	strcpy(mainMemory.memoryArray[pcb.memory_start + 4].data, buffer);

	// mainMemory.memoryArray[pcb.memory_start + 5].name = "state";
	strcpy(mainMemory.memoryArray[pcb.memory_start + 5].name, "state");
	sprintf(buffer, "%s", processToString(pcb.state));
	strcpy(mainMemory.memoryArray[pcb.memory_start + 5].data, buffer);

	strcpy(mainMemory.memoryArray[pcb.memory_start + 6].name, "Var");
	strcpy(mainMemory.memoryArray[pcb.memory_start + 7].name, "Var");
	strcpy(mainMemory.memoryArray[pcb.memory_start + 8].name, "Var");

	FILE* code_file = fopen(filepath, "r");

	if (code_file == NULL) {
		perror("Unable to open file");
	}

	int code_segment_line = 0;

	while (fgets(buffer, 256, code_file)) {

		strcpy(
			mainMemory.memoryArray[pcb.memory_start + CODE_SEGMENT_OFFSET + code_segment_line].name,
			"code");
		// memcpy(mainMemory.memoryArray[pcb.memory_start + CODE_SEGMENT_OFFSET +
		// code_segment_line].data, buffer, 256); printf("Buffer: %c\n", buffer[7]);

		for (int i = 0; i < 256; i++) {
			mainMemory.memoryArray[pcb.memory_start + CODE_SEGMENT_OFFSET + code_segment_line]
				.data[i]
				= buffer[i];
		}

		code_segment_line++;
	}
	code_segment_line--;
	pcb.memory_end = pcb.memory_start + CODE_SEGMENT_OFFSET + code_segment_line;
	printf("PID: %d, Start: %d, Lines of Code: %d, Memory End: %d\n", pcb.id, pcb.memory_start,
		code_segment_line, pcb.memory_end);

	strcpy(mainMemory.memoryArray[pcb.memory_start + 2].name, "mem_end");
	sprintf(buffer, "%d", pcb.memory_end);
	strcpy(mainMemory.memoryArray[pcb.memory_start + 2].data, buffer);

	return pcb;
}

char* processToString(ProcessState state)
{
	switch (state) {
        case READY:
            return "ready";
            break;

        case BLOCKED:
            return "blocked";
            break;

        case RUNNING:
            return "running";
            break;
        case TERMINATED:
            return "terminated";
            break;

        default:
            break;
	}
}

char* resourceToString(Resource resource){
	switch (resource) {
		case USER_INPUT:
			return "User Input";
			break;

		case USER_OUTPUT:
			return "User Output";
			break;

		case FILE_RW:
			return "File";
			break;
		case NIL:
			return "None";
			break;

		default:
			break;
	}
}

void displayMemory(Memory* mainMemory)
{
	printf("Memory Contents:\n");

	for (int i = 0; i < 60; i++) {
		// Check if name and data are valid (not NULL)
		if (mainMemory->memoryArray[i].name != NULL && mainMemory->memoryArray[i].data != NULL) {
			printf("Index %d: { Key: '%s', Value: '%s' }\n", i, mainMemory->memoryArray[i].name,
				mainMemory->memoryArray[i].data);
		} else {
			printf("Index %d: { Key: NULL, Value: NULL }\n", i);
		}
	}
}

// ===============------------------= Schedule Stuff ================

Scheduler* scheduler_new(SchedulerType type, ...)
{
	Scheduler* s = (Scheduler*)malloc(sizeof(Scheduler));
	s->type = type;
	va_list args;
	va_start(args, type);
	if (type == SCHED_ROUND_ROBIN)
		s->rr_quantum = va_arg(args, int);
	else
		s->rr_quantum = 0;
	va_end(args);

	for (int i = 0; i < MLFQ_LEVELS; i++)
		s->input_queues[i] = queue_new();
	s->output_queue = queue_new();
	int base = 1;
	for (int i = 0; i < MLFQ_LEVELS; i++)
		s->mlfq_quantum[i] = base << i; // 1, 2, 4, 8
	s->running_process = NULL;
	return s;
}

void scheduler_add_task(Scheduler* sched, ProcessControlBlock* process)
{
	queue_push_tail(sched->input_queues[0], process);
}
void terminateProcess(ProcessControlBlock *process) {
	if (process == NULL) {
		return;
	}

	process->state = TERMINATED;

	// Clear the memory for that process
	int memory_start = process->memory_start;
	int memory_end = process->memory_end;
	for (int i = memory_start; i <= memory_end; i++) {
		memset(mainMemory.memoryArray[i].data, 0, sizeof(mainMemory.memoryArray[i].data));
		memset(mainMemory.memoryArray[i].name, 0, sizeof(mainMemory.memoryArray[i].name));
	}

	// Find and remove the process from loaded_processes array
	for (int i = 0; i < num_loaded_processes; i++) {
		if (loaded_processes[i] == process || loaded_processes[i]->id == process->id) {
			// Shift the array left
			for (int j = i; j < num_loaded_processes - 1; j++) {
				loaded_processes[j] = loaded_processes[j + 1];
			}
			loaded_processes[num_loaded_processes - 1] = NULL; // Optional: clear the last slot
			num_loaded_processes--;
			break; // Important: exit after removing
		}
	}
	// during termination remove that process if it is locking any resource
	// TODO may be removed but is essential for now .
	if (userInputBlockingProcess != NULL && userInputBlockingProcess->id == process->id) {
		userInputBlockingProcess = NULL;
		safe_sem_post(&userInputSemaphore);
	}
	if (userOutputBlockingProcess != NULL && userOutputBlockingProcess->id == process->id) {
		userOutputBlockingProcess = NULL;
		safe_sem_post(&userOutputSemaphore);
	}
	if (fileBlockingProcess != NULL && fileBlockingProcess->id == process->id) {
		safe_sem_post(&fileSemaphore);
		fileBlockingProcess = NULL;
	}
	update_dashboard();
}

void scheduler_step(Scheduler* sched) {
    int semValue = 0;

    // Check blocked processes first
    if (!queue_is_empty(&blockedQueue) && sched->type != SCHED_FCFS) {
        Queue* temp = queue_new();

        while (!queue_is_empty(&blockedQueue)) {
            ProcessControlBlock* process = (ProcessControlBlock*)queue_pop_head(&blockedQueue);

            switch (process->blockedResource) {
                case FILE_RW:
                    sem_getvalue(&fileSemaphore, &semValue);
                    break;
                case USER_INPUT:
                    sem_getvalue(&userInputSemaphore, &semValue);
                    break;
                case USER_OUTPUT:
                    sem_getvalue(&userOutputSemaphore, &semValue);
                    break;
                default:
                    break;
            }

            if (semValue != 0) {
                process->blockedResource = NIL;
                process->state = READY;
                process->timeslice_used = 0;
                queue_push_tail(sched->input_queues[0], process);
            } else {
                queue_push_tail(temp, process);
            }
        }

        blockedQueue = *temp;
        free(temp); // Free the temporary queue structure
    }

    // Set running process to NULL initially
    sched->running_process = NULL;
	// time_in_queue for each in all processes in queue
	for (int i = 0; i < MLFQ_LEVELS; i++) {
		Queue * currentQ = sched->input_queues[i];
		Queue * tempQ = queue_new();
		if (currentQ!= NULL && !queue_is_empty(currentQ)) {
			while (!queue_is_empty(currentQ)) {
				ProcessControlBlock* process = queue_pop_head(currentQ);
				process->time_in_queue++;
				queue_push_tail(tempQ, process);
			}
		}
		while (!queue_is_empty(tempQ)) {
			ProcessControlBlock* process = queue_pop_head(tempQ);
			queue_push_tail(currentQ, process);
		}
	}

    switch (sched->type) {
        case SCHED_FCFS: {
            if (!queue_is_empty(sched->input_queues[0])) {
                ProcessControlBlock* process = (ProcessControlBlock*)queue_peek_head(sched->input_queues[0]);
                sched->running_process = process; // Update running process

                executeSingleLinePCB(process);

                if (process->memory_start + process->program_counter + CODE_SEGMENT_OFFSET >= process->memory_end) {
                    queue_pop_head(sched->input_queues[0]);
                    queue_push_tail(sched->output_queue, process);
                    sched->running_process = NULL; // Process completed
                	terminateProcess(process);
                }
            }
            break;
        }

        case SCHED_ROUND_ROBIN: {
            if (!queue_is_empty(sched->input_queues[0])) {
                ProcessControlBlock* process = (ProcessControlBlock*)queue_peek_head(sched->input_queues[0]);
                sched->running_process = process; // Update running process

                executeSingleLinePCB(process);
                process->timeslice_used++;

                if (process->state == BLOCKED) {
                    queue_pop_head(sched->input_queues[0]);
                    queue_push_tail(&blockedQueue, process);
                    sched->running_process = NULL; // Process blocked
                    return;
                }

                if (process->memory_start + process->program_counter + CODE_SEGMENT_OFFSET >= process->memory_end) {
                    queue_pop_head(sched->input_queues[0]);
                    queue_push_tail(sched->output_queue, process);
                    sched->running_process = NULL; // Process completed
                	terminateProcess(process);
                } else if (process->timeslice_used >= sched->rr_quantum) {
                    process->timeslice_used = 0;
                    queue_pop_head(sched->input_queues[0]);
                    queue_push_tail(sched->input_queues[0], process);
                    sched->running_process = NULL; // Process moved back to ready queue
                }
            }
            break;
        }

        case SCHED_MLFQ: {
            for (int level = 0; level < MLFQ_LEVELS; level++) {
                if (!queue_is_empty(sched->input_queues[level])) {
                    ProcessControlBlock* process = (ProcessControlBlock*)queue_peek_head(sched->input_queues[level]);
                    sched->running_process = process; // Update running process

                    executeSingleLinePCB(process);
                    process->timeslice_used++;

                    if (process->state == BLOCKED) {
                        queue_pop_head(sched->input_queues[level]);
                        queue_push_tail(&blockedQueue, process);
                        sched->running_process = NULL; // Process blocked
                        return;
                    }

                    if (process->memory_start + process->program_counter + CODE_SEGMENT_OFFSET >= process->memory_end) {
                        process = (ProcessControlBlock*)queue_pop_head(sched->input_queues[level]);
                        process->timeslice_used = 0;
                        queue_push_tail(sched->output_queue, process);
                        sched->running_process = NULL; // Process completed
                    	terminateProcess(process);
                    } else if (process->timeslice_used >= sched->mlfq_quantum[level]) {
                        process = (ProcessControlBlock*)queue_pop_head(sched->input_queues[level]);
                        process->timeslice_used = 0;
                        int next_level = (level < MLFQ_LEVELS - 1) ? level + 1 : level;
                        queue_push_tail(sched->input_queues[next_level], process);
                        sched->running_process = NULL; // Process moved to next level
                    }
                    break;
                }
            }
            break;
        }
    }

    // Update the GUI after each step
    update_queues();
}