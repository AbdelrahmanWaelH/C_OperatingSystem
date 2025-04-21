#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Forward declarations
static void start_simulation(GtkWidget * widget, gpointer data);
static void stop_simulation(GtkWidget * widget, gpointer data);
static void reset_simulation(GtkWidget * widget, gpointer data);
static void step_simulation(GtkWidget * widget, gpointer data);
static void auto_execution(GtkWidget * widget, gpointer data);
static void add_process(GtkWidget * widget, gpointer data);
static void schedule_algorithm_changed(GtkComboBoxText * widget, gpointer data);
static void update_ui(gpointer data);
static void load_process_file(GtkWidget * widget, gpointer data);
static void set_quantum_value(GtkSpinButton * spin_button, gpointer data);

// Global UI elements that need to be updated
typedef struct {
  GtkWidget * process_list;
  GtkWidget * ready_queue;
  GtkWidget * blocked_queue;
  GtkWidget * running_process;
  GtkWidget * memory_grid;
  GtkWidget * execution_log;
  GtkWidget * event_messages;
  GtkWidget * clock_cycle_label;
  GtkWidget * process_count_label;
  GtkWidget * algorithm_label;
  GtkWidget * quantum_spin;
  GtkWidget * mutex_status;
  GtkWidget * scheduler_combo;
  GtkWidget * auto_button;
  guint timer_id;
  gboolean simulation_running;
}
SimulationUI;

// Process representation
typedef enum {
  READY,
  RUNNING,
  BLOCKED
}
ProcessState;

typedef struct {
  int id;
  ProcessState state;
  int priority;
  int memory_start;
  int memory_end;
  int program_counter;
  int arrival_time;
  int time_in_queue;
  char * instructions;
  char * blocked_resource; // NULL if not blocked
}
Process;

// Main application state
typedef struct {
  SimulationUI ui;
  GList * processes;
  Process * running_process;
  GList * ready_queue;
  GList * blocked_queue;
  int clock_cycle;
  char * current_algorithm;
  int quantum;
  gboolean userInput_mutex;
  gboolean userOutput_mutex;
  gboolean file_mutex;
  Process * userInput_owner;
  Process * userOutput_owner;
  Process * file_owner;
  char memory[60][20]; // 60 memory words, each can hold a string up to 20 chars
}
SimulationState;

// Initialize the simulation state
SimulationState * init_simulation_state() {
  SimulationState * state = g_malloc(sizeof(SimulationState));
  state -> processes = NULL;
  state -> running_process = NULL;
  state -> ready_queue = NULL;
  state -> blocked_queue = NULL;
  state -> clock_cycle = 0;
  state -> current_algorithm = g_strdup("FCFS");
  state -> quantum = 2;
  state -> userInput_mutex = FALSE;
  state -> userOutput_mutex = FALSE;
  state -> file_mutex = FALSE;
  state -> userInput_owner = NULL;
  state -> userOutput_owner = NULL;
  state -> file_owner = NULL;

  // Initialize memory
  for (int i = 0; i < 60; i++) {
    strcpy(state -> memory[i], "EMPTY");
  }

  return state;
}

static void apply_global_css(void) {
  // GtkCssProvider *provider = gtk_css_provider_new();
  // gtk_css_provider_load_from_data(provider,
  //     /* Your CSS from the first message */
  //     "window { background-color: #1e2229; }"
  //   ".main-container { background-color: #262c36; border-radius: 8px; margin: 8px; transition: all 0.3s ease; }"
  //   ".main-container:hover { box-shadow: 0 0 10px rgba(103, 232, 249, 0.3); }"
  //   ".header { font-size: 18px; font-weight: bold; color: #67e8f9; margin: 8px; padding-bottom: 5px; border-bottom: 1px solid #556; }"

  //   ".dashboard-card { background-color: #2d343f; border-radius: 6px; padding: 10px; margin: 5px; transition: transform 0.2s; }"
  //   ".dashboard-card:hover { transform: translateY(-2px); background-color: #323a47; }"

  //   ".process-row { padding: 4px; border-bottom: 1px solid #444; transition: background-color 0.2s; }"
  //   ".process-row:hover { background-color: #3a4454; }"

  //   ".running { background-color: rgba(62, 207, 142, 0.2); border-left: 3px solid #3ecf8e; }"
  //   ".blocked { background-color: rgba(240, 62, 62, 0.1); border-left: 3px solid #f03e3e; }"
  //   ".ready { background-color: rgba(82, 138, 224, 0.1); border-left: 3px solid #528ae0; }"

  //   ".memory-cell { background-color: #2a313c; border: 1px solid #444; border-radius: 3px; padding: 5px; margin: 1px; transition: all 0.2s; }"
  //   ".memory-cell.used { background-color: #39496a; border-color: #5d7cb8; }"
  //   ".memory-cell:hover { transform: scale(1.05); box-shadow: 0 0 5px rgba(103, 232, 249, 0.5); }"

  //   ".control-button { background-color: #2b5a83; color: white; border-radius: 4px; padding: 8px 15px; border: none; transition: all 0.3s; }"
  //   ".control-button:hover { background-color: #3a75aa; transform: translateY(-2px); box-shadow: 0 3px 5px rgba(0,0,0,0.2); }"

  //   ".start-button { background-color: #2b7a4b; }"
  //   ".start-button:hover { background-color: #3a9a64; }"

  //   ".stop-button { background-color: #7a2b2b; }"
  //   ".stop-button:hover { background-color: #9a3a3a; }"

  //   ".reset-button { background-color: #7a5c2b; }"
  //   ".reset-button:hover { background-color: #9a753a; }"

  //   ".step-button { background-color: #2b3d7a; }"
  //   ".step-button:hover { background-color: #3a50aa; }"

  //   ".pulse-animation { animation: pulse 2s infinite; }"
  //   "@keyframes pulse {"
  //   "  0% { opacity: 1; }"
  //   "  50% { opacity: 0.6; }"
  //   "  100% { opacity: 1; }"
  //   "}"

  //   ".log-entry { border-bottom: 1px solid #444; padding: 3px 0; animation: fadeIn 0.5s; }"
  //   "@keyframes fadeIn {"
  //   "  from { opacity: 0; transform: translateY(-10px); }"
  //   "  to { opacity: 1; transform: translateY(0); }"
  //   "}"

  //   ".status-label { color: #adb5bd; font-size: 12px; }"
  //   ".value-label { color: #e9ecef; font-weight: bold; }"

  //   ".resource-available { color: #40c057; }"
  //   ".resource-locked { color: #fa5252; }"

  //   ".combo-box { background-color: #2d343f; border: 1px solid #444; border-radius: 4px; color: white; }"
  //   ".scrolled { border: 1px solid #444; border-radius: 4px; }"

  //   ".active-container { animation: highlight 1s; }"
  //   "@keyframes highlight {"
  //   "  0% { background-color: rgba(103, 232, 249, 0.1); }"
  //   "  100% { background-color: transparent; }"
  //   "}"

  //   /* Tooltips */
  //   "tooltip { background-color: #1e2229; color: #e9ecef; }"

  //   /* Transitions for switching between panels */
  //   ".panel-transition { transition: opacity 0.3s, transform 0.3s; }"
  //   ".panel-hidden { opacity: 0; transform: translateX(-20px); }"
  //   ".panel-visible { opacity: 1; transform: translateX(0); }",
  //     -1, NULL);

  // // Apply to the entire application
  // GdkScreen *screen = gdk_screen_get_default();
  // gtk_style_context_add_provider_for_screen(screen,
  //                                        GTK_STYLE_PROVIDER(provider),
  //                                        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  // g_object_unref(provider);
}


static void update_process_list(SimulationState * state) {
  // Clear existing rows
  GtkListBox * list_box = GTK_LIST_BOX(state -> ui.process_list);
  GList * children, * iter;
  children = gtk_container_get_children(GTK_CONTAINER(list_box));
  for (iter = children; iter != NULL; iter = g_list_next(iter))
    gtk_widget_destroy(GTK_WIDGET(iter -> data));
  g_list_free(children);

  // Add process rows
  for (GList * proc_iter = state -> processes; proc_iter != NULL; proc_iter = proc_iter -> next) {
    Process * proc = (Process * ) proc_iter -> data;

    GtkWidget * row = gtk_list_box_row_new();
    GtkWidget * hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_add(GTK_CONTAINER(row), hbox);

    // Add process details
    char pid_str[32], state_str[32], priority_str[32], memory_str[32], pc_str[32];
    sprintf(pid_str, "%d", proc -> id);

    const char * state_names[] = {
      "READY",
      "RUNNING",
      "BLOCKED"
    };
    sprintf(state_str, "%s", state_names[proc -> state]);

    sprintf(priority_str, "%d", proc -> priority);
    sprintf(memory_str, "%d-%d", proc -> memory_start, proc -> memory_end);
    sprintf(pc_str, "%d", proc -> program_counter);

    GtkWidget * pid_label = gtk_label_new(pid_str);
    gtk_widget_set_size_request(pid_label, 40, -1);
    gtk_box_pack_start(GTK_BOX(hbox), pid_label, FALSE, FALSE, 5);

    GtkWidget * state_label = gtk_label_new(state_str);
    gtk_widget_set_size_request(state_label, 80, -1);
    gtk_box_pack_start(GTK_BOX(hbox), state_label, FALSE, FALSE, 5);

    GtkWidget * priority_label = gtk_label_new(priority_str);
    gtk_widget_set_size_request(priority_label, 60, -1);
    gtk_box_pack_start(GTK_BOX(hbox), priority_label, FALSE, FALSE, 5);

    GtkWidget * memory_label = gtk_label_new(memory_str);
    gtk_widget_set_size_request(memory_label, 80, -1);
    gtk_box_pack_start(GTK_BOX(hbox), memory_label, FALSE, FALSE, 5);

    GtkWidget * pc_label = gtk_label_new(pc_str);
    gtk_widget_set_size_request(pc_label, 40, -1);
    gtk_box_pack_start(GTK_BOX(hbox), pc_label, FALSE, FALSE, 5);

    // Set row style based on process state
    GtkStyleContext * context = gtk_widget_get_style_context(row);
    gtk_style_context_add_class(context, "process-row");

    if (proc -> state == RUNNING)
      gtk_style_context_add_class(context, "running");
    else if (proc -> state == BLOCKED)
      gtk_style_context_add_class(context, "blocked");
    else
      gtk_style_context_add_class(context, "ready");

    gtk_widget_show_all(row);
    gtk_list_box_insert(list_box, row, -1);
  }
}

static void update_queue_displays(SimulationState * state) {
  // Update Ready Queue
  GtkListBox * ready_list = GTK_LIST_BOX(state -> ui.ready_queue);
  GList * children, * iter;

  children = gtk_container_get_children(GTK_CONTAINER(ready_list));
  for (iter = children; iter != NULL; iter = g_list_next(iter))
    gtk_widget_destroy(GTK_WIDGET(iter -> data));
  g_list_free(children);

  for (GList * q_iter = state -> ready_queue; q_iter != NULL; q_iter = q_iter -> next) {
    Process * proc = (Process * ) q_iter -> data;

    GtkWidget * row = gtk_list_box_row_new();
    GtkWidget * hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_add(GTK_CONTAINER(row), hbox);

    char pid_str[32], instr_str[64], time_str[32];
    sprintf(pid_str, "PID: %d", proc -> id);
    sprintf(instr_str, "Next: %s", proc -> instructions ? proc -> instructions : "N/A");
    sprintf(time_str, "Time: %d", proc -> time_in_queue);

    gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(pid_str), FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(instr_str), FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(time_str), FALSE, FALSE, 5);

    GtkStyleContext * context = gtk_widget_get_style_context(row);
    gtk_style_context_add_class(context, "process-row");
    gtk_style_context_add_class(context, "ready");

    gtk_widget_show_all(row);
    gtk_list_box_insert(ready_list, row, -1);
  }

  // Update Blocked Queue with same approach
  GtkListBox * blocked_list = GTK_LIST_BOX(state -> ui.blocked_queue);

  children = gtk_container_get_children(GTK_CONTAINER(blocked_list));
  for (iter = children; iter != NULL; iter = g_list_next(iter))
    gtk_widget_destroy(GTK_WIDGET(iter -> data));
  g_list_free(children);

  for (GList * q_iter = state -> blocked_queue; q_iter != NULL; q_iter = q_iter -> next) {
    Process * proc = (Process * ) q_iter -> data;

    GtkWidget * row = gtk_list_box_row_new();
    GtkWidget * hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_add(GTK_CONTAINER(row), hbox);

    char pid_str[32], res_str[64], time_str[32];
    sprintf(pid_str, "PID: %d", proc -> id);
    sprintf(res_str, "Waiting: %s", proc -> blocked_resource ? proc -> blocked_resource : "Unknown");
    sprintf(time_str, "Time: %d", proc -> time_in_queue);

    gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(pid_str), FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(res_str), FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(time_str), FALSE, FALSE, 5);

    GtkStyleContext * context = gtk_widget_get_style_context(row);
    gtk_style_context_add_class(context, "process-row");
    gtk_style_context_add_class(context, "blocked");

    gtk_widget_show_all(row);
    gtk_list_box_insert(blocked_list, row, -1);
  }

  // Update Running Process display
  GtkContainer * running_container = GTK_CONTAINER(state -> ui.running_process);

  children = gtk_container_get_children(running_container);
  for (iter = children; iter != NULL; iter = g_list_next(iter))
    gtk_widget_destroy(GTK_WIDGET(iter -> data));
  g_list_free(children);

  if (state -> running_process) {
    Process * proc = state -> running_process;

    GtkWidget * vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    char pid_str[32], instr_str[128], pc_str[32];
    sprintf(pid_str, "Process ID: %d", proc -> id);
    sprintf(instr_str, "Current Instruction: %s",
      proc -> instructions ? proc -> instructions : "N/A");
    sprintf(pc_str, "Program Counter: %d", proc -> program_counter);

    GtkWidget * pid_label = gtk_label_new(pid_str);
    gtk_label_set_xalign(GTK_LABEL(pid_label), 0);

    GtkWidget * instr_label = gtk_label_new(instr_str);
    gtk_label_set_xalign(GTK_LABEL(instr_label), 0);

    GtkWidget * pc_label = gtk_label_new(pc_str);
    gtk_label_set_xalign(GTK_LABEL(pc_label), 0);

    gtk_box_pack_start(GTK_BOX(vbox), pid_label, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(vbox), instr_label, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(vbox), pc_label, FALSE, FALSE, 2);

    GtkStyleContext * context = gtk_widget_get_style_context(vbox);
    gtk_style_context_add_class(context, "running");
    gtk_style_context_add_class(context, "pulse-animation");

    gtk_container_add(running_container, vbox);
    gtk_widget_show_all(vbox);
  } else {
    GtkWidget * no_proc = gtk_label_new("No process running");
    gtk_container_add(running_container, no_proc);
    gtk_widget_show(no_proc);
  }
}

static void update_memory_grid(SimulationState * state) {
  GtkGrid * grid = GTK_GRID(state -> ui.memory_grid);

  // Remove all children
  GList * children, * iter;
  children = gtk_container_get_children(GTK_CONTAINER(grid));
  for (iter = children; iter != NULL; iter = g_list_next(iter))
    gtk_widget_destroy(GTK_WIDGET(iter -> data));
  g_list_free(children);

  // Create 6x10 grid (60 memory cells)
  for (int row = 0; row < 6; row++) {
    for (int col = 0; col < 10; col++) {
      int index = row * 10 + col;

      GtkWidget * frame = gtk_frame_new(NULL);
      GtkWidget * label = gtk_label_new(state -> memory[index]);

      // Add address as frame label
      char addr_str[8];
      sprintf(addr_str, "%d", index);
      gtk_frame_set_label(GTK_FRAME(frame), addr_str);

      // Style the cell
      gtk_container_add(GTK_CONTAINER(frame), label);
      gtk_widget_set_size_request(frame, 75, 30);

      GtkStyleContext * context = gtk_widget_get_style_context(frame);
      gtk_style_context_add_class(context, "memory-cell");

      // Add "used" class if the memory cell is not empty
      if (strcmp(state -> memory[index], "EMPTY") != 0) {
        gtk_style_context_add_class(context, "used");
      }

      gtk_grid_attach(grid, frame, col, row, 1, 1);
    }
  }

  gtk_widget_show_all(GTK_WIDGET(grid));
}

static void update_mutex_status(SimulationState * state) {
  GtkContainer * container = GTK_CONTAINER(state -> ui.mutex_status);

  // Clear existing content
  GList * children, * iter;
  children = gtk_container_get_children(container);
  for (iter = children; iter != NULL; iter = g_list_next(iter))
    gtk_widget_destroy(GTK_WIDGET(iter -> data));
  g_list_free(children);

  GtkWidget * vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

  // userInput mutex
  GtkWidget * input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  GtkWidget * input_label = gtk_label_new("userInput:");
  gtk_widget_set_size_request(input_label, 100, -1);
  gtk_box_pack_start(GTK_BOX(input_box), input_label, FALSE, FALSE, 0);

  GtkWidget * input_status;
  if (state -> userInput_mutex) {
    char status_str[64];
    sprintf(status_str, "LOCKED by PID %d", state -> userInput_owner ? state -> userInput_owner -> id : -1);
    input_status = gtk_label_new(status_str);
    GtkStyleContext * context = gtk_widget_get_style_context(input_status);
    gtk_style_context_add_class(context, "resource-locked");
  } else {
    input_status = gtk_label_new("AVAILABLE");
    GtkStyleContext * context = gtk_widget_get_style_context(input_status);
    gtk_style_context_add_class(context, "resource-available");
  }
  gtk_box_pack_start(GTK_BOX(input_box), input_status, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), input_box, FALSE, FALSE, 0);

  // Similar for userOutput mutex
  GtkWidget * output_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  GtkWidget * output_label = gtk_label_new("userOutput:");
  gtk_widget_set_size_request(output_label, 100, -1);
  gtk_box_pack_start(GTK_BOX(output_box), output_label, FALSE, FALSE, 0);

  GtkWidget * output_status;
  if (state -> userOutput_mutex) {
    char status_str[64];
    sprintf(status_str, "LOCKED by PID %d", state -> userOutput_owner ? state -> userOutput_owner -> id : -1);
    output_status = gtk_label_new(status_str);
    GtkStyleContext * context = gtk_widget_get_style_context(output_status);
    gtk_style_context_add_class(context, "resource-locked");
  } else {
    output_status = gtk_label_new("AVAILABLE");
    GtkStyleContext * context = gtk_widget_get_style_context(output_status);
    gtk_style_context_add_class(context, "resource-available");
  }
  gtk_box_pack_start(GTK_BOX(output_box), output_status, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), output_box, FALSE, FALSE, 0);

  // And for file mutex
  GtkWidget * file_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  GtkWidget * file_label = gtk_label_new("file:");
  gtk_widget_set_size_request(file_label, 100, -1);
  gtk_box_pack_start(GTK_BOX(file_box), file_label, FALSE, FALSE, 0);

  GtkWidget * file_status;
  if (state -> file_mutex) {
    char status_str[64];
    sprintf(status_str, "LOCKED by PID %d", state -> file_owner ? state -> file_owner -> id : -1);
    file_status = gtk_label_new(status_str);
    GtkStyleContext * context = gtk_widget_get_style_context(file_status);
    gtk_style_context_add_class(context, "resource-locked");
  } else {
    file_status = gtk_label_new("AVAILABLE");
    GtkStyleContext * context = gtk_widget_get_style_context(file_status);
    gtk_style_context_add_class(context, "resource-available");
  }
  gtk_box_pack_start(GTK_BOX(file_box), file_status, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), file_box, FALSE, FALSE, 0);

  gtk_container_add(container, vbox);
  gtk_widget_show_all(GTK_WIDGET(container));
}

static void add_log_entry(GtkWidget * log_view,const char * entry) {
  GtkWidget * row = gtk_list_box_row_new();
  GtkWidget * label = gtk_label_new(entry);
  gtk_label_set_xalign(GTK_LABEL(label), 0); // Left align text
  gtk_container_add(GTK_CONTAINER(row), label);

  GtkStyleContext * context = gtk_widget_get_style_context(row);
  gtk_style_context_add_class(context, "log-entry");

  // Insert at top
  gtk_list_box_insert(GTK_LIST_BOX(log_view), row, 0);
  gtk_widget_show_all(row);

  // Limit log entries to prevent performance issues
  GList * children = gtk_container_get_children(GTK_CONTAINER(log_view));
  int count = g_list_length(children);
  if (count > 100) {
    GtkWidget * last = GTK_WIDGET(g_list_last(children) -> data);
    gtk_widget_destroy(last);
  }
  g_list_free(children);
}

static void update_ui(gpointer data) {
  SimulationState * state = (SimulationState * ) data;

  // Update the key components
  update_process_list(state);
  update_queue_displays(state);
  update_memory_grid(state);
  update_mutex_status(state);

  // Update summary labels
  char cycle_str[32], process_str[32];
  sprintf(cycle_str, "%d", state -> clock_cycle);
  sprintf(process_str, "%d", g_list_length(state -> processes));

  gtk_label_set_text(GTK_LABEL(state -> ui.clock_cycle_label), cycle_str);
  gtk_label_set_text(GTK_LABEL(state -> ui.process_count_label), process_str);
  gtk_label_set_text(GTK_LABEL(state -> ui.algorithm_label), state -> current_algorithm);
}

static gboolean auto_execution_handler(gpointer data) {
  SimulationState * state = (SimulationState * ) data;

  if (!state -> ui.simulation_running) {
    return FALSE; // Stop the timer
  }

  // Simulate one step of execution
  state -> clock_cycle++;

  // Add log entry to show something happened
  char log_str[256];
  sprintf(log_str, "Cycle %d: Executed instruction", state -> clock_cycle);
  add_log_entry(state -> ui.execution_log, log_str);

  // Mockup: Randomly change some state
  if (state -> processes != NULL && g_random_int_range(0, 10) > 7) {
    Process * proc = g_list_nth_data(state -> processes, g_random_int_range(0, g_list_length(state -> processes)));
    if (proc) {
      // Change state for demonstration
      int old_state = proc -> state;
      proc -> state = g_random_int_range(0, 3); // Random state

      const char * state_names[] = {
        "READY",
        "RUNNING",
        "BLOCKED"
      };
      sprintf(log_str, "Process %d changed from %s to %s",
        proc -> id, state_names[old_state], state_names[proc -> state]);

      add_log_entry(state -> ui.event_messages, log_str);

      // Update process queues for demonstration
      if (proc -> state == RUNNING) {
        state -> running_process = proc;
      } else if (proc -> state == BLOCKED) {
        // Randomly assign blocked resource
        const char * resources[] = {
          "userInput",
          "userOutput",
          "file"
        };
        proc -> blocked_resource = g_strdup(resources[g_random_int_range(0, 3)]);

        // Make sure it's in blocked queue
        if (!g_list_find(state -> blocked_queue, proc)) {
          state -> blocked_queue = g_list_append(state -> blocked_queue, proc);
        }

        // Remove from ready queue if present
        state -> ready_queue = g_list_remove(state -> ready_queue, proc);

        sprintf(log_str, "Process %d is blocked waiting for %s",
          proc -> id, proc -> blocked_resource);
        add_log_entry(state -> ui.event_messages, log_str);
      } else if (proc -> state == READY) {
        // Make sure it's in ready queue
        if (!g_list_find(state -> ready_queue, proc)) {
          state -> ready_queue = g_list_append(state -> ready_queue, proc);
        }

        // Remove from blocked queue if present
        state -> blocked_queue = g_list_remove(state -> blocked_queue, proc);

        // Free the blocked resource name if it exists
        if (proc -> blocked_resource) {
          g_free(proc -> blocked_resource);
          proc -> blocked_resource = NULL;
        }
      }
    }
  }

  // Randomly modify memory for demonstration
  if (g_random_int_range(0, 10) > 8) {
    int mem_index = g_random_int_range(0, 60);
    if (strcmp(state -> memory[mem_index], "EMPTY") == 0) {
      sprintf(state -> memory[mem_index], "PROC-%d", g_random_int_range(1, 10));

      sprintf(log_str, "Memory word %d allocated", mem_index);
      add_log_entry(state -> ui.event_messages, log_str);
    } else {
      strcpy(state -> memory[mem_index], "EMPTY");

      sprintf(log_str, "Memory word %d freed", mem_index);
      add_log_entry(state -> ui.event_messages, log_str);
    }
  }

  // Update the UI with new state
  update_ui(state);

  // Continue the timer
  return TRUE;
}

static void add_mock_process(SimulationState * state) {
  // Create a mock process for demonstration
  static int next_pid = 1;
  Process * proc = g_malloc(sizeof(Process));

  proc -> id = next_pid++;
  proc -> state = READY;
  proc -> priority = g_random_int_range(1, 10);
  proc -> memory_start = g_random_int_range(0, 40);
  proc -> memory_end = proc -> memory_start + g_random_int_range(5, 15);
  if (proc -> memory_end >= 60) proc -> memory_end = 59;
  proc -> program_counter = 0;
  proc -> arrival_time = state -> clock_cycle;
  proc -> time_in_queue = 0;
  proc -> instructions = g_strdup("SAMPLE INSTRUCTION");
  proc -> blocked_resource = NULL;

  // Add to processes list
  state -> processes = g_list_append(state -> processes, proc);

  // Add to ready queue
  state -> ready_queue = g_list_append(state -> ready_queue, proc);

  // Allocate memory
  for (int i = proc -> memory_start; i <= proc -> memory_end; i++) {
    sprintf(state -> memory[i], "P%d", proc -> id);
  }

  // Add log entry
  char log_str[128];
  sprintf(log_str, "Process %d created and added to ready queue", proc -> id);
  add_log_entry(state -> ui.event_messages, log_str);

  update_ui(state);
}

static void start_simulation(GtkWidget * widget, gpointer data) {
  SimulationState * state = (SimulationState * ) data;

  if (!state -> ui.simulation_running) {
    state -> ui.simulation_running = TRUE;
    state -> ui.timer_id = g_timeout_add(1000, auto_execution_handler, state); // 1 second interval

    add_log_entry(state -> ui.event_messages, "Simulation started");

    // Disable start button, enable stop button
    gtk_widget_set_sensitive(widget, FALSE);
    gtk_widget_set_sensitive(state -> ui.auto_button, FALSE);

    // Find the stop button and enable it
    GList * children = gtk_container_get_children(GTK_CONTAINER(gtk_widget_get_parent(widget)));
    for (GList * iter = children; iter != NULL; iter = g_list_next(iter)) {
      if (GTK_IS_BUTTON(iter -> data) &&
        g_strcmp0(gtk_button_get_label(GTK_BUTTON(iter -> data)), "Stop") == 0) {
        gtk_widget_set_sensitive(GTK_WIDGET(iter -> data), TRUE);
        break;
      }
    }
    g_list_free(children);
  }
}

static void stop_simulation(GtkWidget * widget, gpointer data) {
  SimulationState * state = (SimulationState * ) data;

  if (state -> ui.simulation_running) {
    state -> ui.simulation_running = FALSE;
    if (state -> ui.timer_id > 0) {
      g_source_remove(state -> ui.timer_id);
      state -> ui.timer_id = 0;
    }

    add_log_entry(state -> ui.event_messages, "Simulation stopped");

    // Disable stop button, enable start button
    gtk_widget_set_sensitive(widget, FALSE);

    // Find the start button and enable it
    GList * children = gtk_container_get_children(GTK_CONTAINER(gtk_widget_get_parent(widget)));
    for (GList * iter = children; iter != NULL; iter = g_list_next(iter)) {
      if (GTK_IS_BUTTON(iter -> data) &&
        g_strcmp0(gtk_button_get_label(GTK_BUTTON(iter -> data)), "Start") == 0) {
        gtk_widget_set_sensitive(GTK_WIDGET(iter -> data), TRUE);
        break;
      }
    }

    // Re-enable auto execution button
    gtk_widget_set_sensitive(state -> ui.auto_button, TRUE);

    g_list_free(children);
  }
}

static void reset_simulation(GtkWidget * widget, gpointer data) {
  SimulationState * state = (SimulationState * ) data;

  // First stop if running
  if (state -> ui.simulation_running) {
    stop_simulation(widget, data);
  }

  // Reset clock cycle
  state -> clock_cycle = 0;

  // Free all processes
  for (GList * iter = state -> processes; iter != NULL; iter = g_list_next(iter)) {
    Process * proc = (Process * ) iter -> data;
    if (proc -> instructions) g_free(proc -> instructions);
    if (proc -> blocked_resource) g_free(proc -> blocked_resource);
    g_free(proc);
  }
  g_list_free(state -> processes);
  state -> processes = NULL;

  // Clear queues
  g_list_free(state -> ready_queue);
  g_list_free(state -> blocked_queue);
  state -> ready_queue = NULL;
  state -> blocked_queue = NULL;
  state -> running_process = NULL;

  // Reset memory
  for (int i = 0; i < 60; i++) {
    strcpy(state -> memory[i], "EMPTY");
  }

  // Reset mutex states
  state -> userInput_mutex = FALSE;
  state -> userOutput_mutex = FALSE;
  state -> file_mutex = FALSE;
  state -> userInput_owner = NULL;
  state -> userOutput_owner = NULL;
  state -> file_owner = NULL;

  add_log_entry(state -> ui.event_messages, "Simulation reset");

  // Update UI
  update_ui(state);

  // Enable all control buttons
  gtk_widget_set_sensitive(state -> ui.auto_button, TRUE);

  // Find and enable the start button
  GList * children = gtk_container_get_children(GTK_CONTAINER(gtk_widget_get_parent(widget)));
  for (GList * iter = children; iter != NULL; iter = g_list_next(iter)) {
    if (GTK_IS_BUTTON(iter -> data) &&
      g_strcmp0(gtk_button_get_label(GTK_BUTTON(iter -> data)), "Start") == 0) {
      gtk_widget_set_sensitive(GTK_WIDGET(iter -> data), TRUE);
      break;
    }
  }
  g_list_free(children);
}

static void step_simulation(GtkWidget * widget, gpointer data) {
  SimulationState * state = (SimulationState * ) data;

  // Execute one step of simulation
  state -> clock_cycle++;

  // Add log entry to show something happened
  char log_str[256];
  sprintf(log_str, "Cycle %d: Executed one step", state -> clock_cycle);
  add_log_entry(state -> ui.execution_log, log_str);

  // For demonstration, let's randomly change a process state
  if (state -> processes != NULL) {
    Process * proc = g_list_nth_data(state -> processes, g_random_int_range(0, g_list_length(state -> processes)));
    if (proc) {
      // Change state for demonstration
      int old_state = proc -> state;
      proc -> state = g_random_int_range(0, 3); // Random state

      const char * state_names[] = {
        "READY",
        "RUNNING",
        "BLOCKED"
      };
      sprintf(log_str, "Process %d changed from %s to %s",
        proc -> id, state_names[old_state], state_names[proc -> state]);

      add_log_entry(state -> ui.event_messages, log_str);
    }
  }

  // Update the UI with new state
  update_ui(state);
}

static void auto_execution(GtkWidget * widget, gpointer data) {
  SimulationState * state = (SimulationState * ) data;

  if (!state -> ui.simulation_running) {
    // Execute several steps automatically
    for (int i = 0; i < 5; i++) {
      step_simulation(widget, data);
      // Add a small delay for visual effect
      while (gtk_events_pending()) gtk_main_iteration();
      g_usleep(200000); // 200ms delay
    }
  }
}

static void schedule_algorithm_changed(GtkComboBoxText * widget, gpointer data) {
  SimulationState * state = (SimulationState * ) data;

  // Get the selected algorithm
  char * selected = gtk_combo_box_text_get_active_text(widget);
  if (selected) {
    // Update the algorithm
    if (state -> current_algorithm) g_free(state -> current_algorithm);
    state -> current_algorithm = g_strdup(selected);

    char log_str[128];
    sprintf(log_str, "Scheduling algorithm changed to %s", selected);
    add_log_entry(state -> ui.event_messages, log_str);

    // Update the UI (specifically the algorithm label)
    gtk_label_set_text(GTK_LABEL(state -> ui.algorithm_label), state -> current_algorithm);

    // Enable/disable quantum spin button based on selection
    if (g_strcmp0(selected, "Round Robin") == 0) {
      gtk_widget_set_sensitive(state -> ui.quantum_spin, TRUE);
    } else {
      gtk_widget_set_sensitive(state -> ui.quantum_spin, FALSE);
    }

    g_free(selected);
  }
}

static void set_quantum_value(GtkSpinButton * spin_button, gpointer data) {
  SimulationState * state = (SimulationState * ) data;

  // Get the value from the spin button
  int quantum = gtk_spin_button_get_value_as_int(spin_button);
  state -> quantum = quantum;

  char log_str[128];
  sprintf(log_str, "Round Robin quantum set to %d", quantum);
  add_log_entry(state -> ui.event_messages, log_str);
}

static void add_process(GtkWidget * widget, gpointer data) {
  SimulationState * state = (SimulationState * ) data;
  add_mock_process(state);
}

static void load_process_file(GtkWidget * widget, gpointer data) {
  SimulationState * state = (SimulationState * ) data;

  GtkWidget * dialog;
  GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
  gint res;

  dialog = gtk_file_chooser_dialog_new("Open Process File",
    GTK_WINDOW(gtk_widget_get_toplevel(widget)),
    action,
    "_Cancel", GTK_RESPONSE_CANCEL,
    "_Open", GTK_RESPONSE_ACCEPT,
    NULL);

  res = gtk_dialog_run(GTK_DIALOG(dialog));
  if (res == GTK_RESPONSE_ACCEPT) {
    char * filename;
    GtkFileChooser * chooser = GTK_FILE_CHOOSER(dialog);
    filename = gtk_file_chooser_get_filename(chooser);

    // For demonstration, just add a mock process
    add_mock_process(state);

    char log_str[256];
    sprintf(log_str, "Loaded process file: %s", filename);
    add_log_entry(state -> ui.event_messages, log_str);

    g_free(filename);
  }

  gtk_widget_destroy(dialog);
}

static GtkWidget* build_ui(SimulationState * state) {
  GtkWidget * window;
  GtkWidget * main_box;
  GtkWidget * dashboard_box, * resource_box, * control_box, * log_box;
  GtkWidget * frame, * label, * scrolled_window, * box, * grid;
  GtkWidget * button, * combo_box, * spin_button;

  // Create main window
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "OS Scheduler Simulation");
  gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);
  gtk_container_set_border_width(GTK_CONTAINER(window), 10);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);


  // Main container
  main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_add(GTK_CONTAINER(window), main_box);

  // Dashboard Section (Top)
  dashboard_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_box_pack_start(GTK_BOX(main_box), dashboard_box, FALSE, TRUE, 0);

  // Overview Card
  frame = gtk_frame_new("System Overview");
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);
  gtk_box_pack_start(GTK_BOX(dashboard_box), frame, TRUE, TRUE, 0);

  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_container_add(GTK_CONTAINER(frame), box);

  // Add status labels
  GtkWidget * overview_grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(overview_grid), 5);
  gtk_grid_set_column_spacing(GTK_GRID(overview_grid), 10);
  gtk_box_pack_start(GTK_BOX(box), overview_grid, TRUE, TRUE, 5);

  // Clock Cycle
  label = gtk_label_new("Clock Cycle:");
  gtk_widget_set_halign(label, GTK_ALIGN_START);
  GtkStyleContext * context = gtk_widget_get_style_context(label);
  gtk_style_context_add_class(context, "status-label");
  gtk_grid_attach(GTK_GRID(overview_grid), label, 0, 0, 1, 1);

  state -> ui.clock_cycle_label = gtk_label_new("0");
  gtk_widget_set_halign(state -> ui.clock_cycle_label, GTK_ALIGN_START);
  context = gtk_widget_get_style_context(state -> ui.clock_cycle_label);
  gtk_style_context_add_class(context, "value-label");
  gtk_grid_attach(GTK_GRID(overview_grid), state -> ui.clock_cycle_label, 1, 0, 1, 1);

  // Process Count
  label = gtk_label_new("Processes:");
  gtk_widget_set_halign(label, GTK_ALIGN_START);
  context = gtk_widget_get_style_context(label);
  gtk_style_context_add_class(context, "status-label");
  gtk_grid_attach(GTK_GRID(overview_grid), label, 0, 1, 1, 1);

  state -> ui.process_count_label = gtk_label_new("0");
  gtk_widget_set_halign(state -> ui.process_count_label, GTK_ALIGN_START);
  context = gtk_widget_get_style_context(state -> ui.process_count_label);
  gtk_style_context_add_class(context, "value-label");
  gtk_grid_attach(GTK_GRID(overview_grid), state -> ui.process_count_label, 1, 1, 1, 1);

  // Algorithm
  label = gtk_label_new("Algorithm:");
  gtk_widget_set_halign(label, GTK_ALIGN_START);
  context = gtk_widget_get_style_context(label);
  gtk_style_context_add_class(context, "status-label");
  gtk_grid_attach(GTK_GRID(overview_grid), label, 0, 2, 1, 1);

  state -> ui.algorithm_label = gtk_label_new("FCFS");
  gtk_widget_set_halign(state -> ui.algorithm_label, GTK_ALIGN_START);
  context = gtk_widget_get_style_context(state -> ui.algorithm_label);
  gtk_style_context_add_class(context, "value-label");
  gtk_grid_attach(GTK_GRID(overview_grid), state -> ui.algorithm_label, 1, 2, 1, 1);

  // Style the overview frame
  context = gtk_widget_get_style_context(frame);
  gtk_style_context_add_class(context, "dashboard-card");

  // Scheduler Control Card
  frame = gtk_frame_new("Scheduler Control");
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);
  gtk_box_pack_start(GTK_BOX(dashboard_box), frame, TRUE, TRUE, 0);

  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_container_add(GTK_CONTAINER(frame), box);

  // Algorithm selector
  GtkWidget * alg_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(box), alg_box, FALSE, FALSE, 5);

  label = gtk_label_new("Algorithm:");
  gtk_box_pack_start(GTK_BOX(alg_box), label, FALSE, FALSE, 0);

  combo_box = gtk_combo_box_text_new();
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box), "FCFS");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box), "Round Robin");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box), "Multilevel Feedback");
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo_box), 0);
  gtk_box_pack_start(GTK_BOX(alg_box), combo_box, TRUE, TRUE, 0);
  state -> ui.scheduler_combo = combo_box;
  g_signal_connect(combo_box, "changed", G_CALLBACK(schedule_algorithm_changed), state);

  // Quantum setting
  GtkWidget * quantum_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(box), quantum_box, FALSE, FALSE, 5);

  label = gtk_label_new("Quantum:");
  gtk_box_pack_start(GTK_BOX(quantum_box), label, FALSE, FALSE, 0);

  spin_button = gtk_spin_button_new_with_range(1, 10, 1);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_button), 2);
  gtk_box_pack_start(GTK_BOX(quantum_box), spin_button, TRUE, TRUE, 0);
  state -> ui.quantum_spin = spin_button;
  g_signal_connect(spin_button, "value-changed", G_CALLBACK(set_quantum_value), state);
  gtk_widget_set_sensitive(spin_button, FALSE);

  // Control buttons
  GtkWidget * button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(box), button_box, TRUE, TRUE, 5);

  button = gtk_button_new_with_label("Start");
  context = gtk_widget_get_style_context(button);
  gtk_style_context_add_class(context, "control-button");
  gtk_style_context_add_class(context, "start-button");
  gtk_box_pack_start(GTK_BOX(button_box), button, TRUE, TRUE, 0);
  g_signal_connect(button, "clicked", G_CALLBACK(start_simulation), state);

  button = gtk_button_new_with_label("Stop");
  context = gtk_widget_get_style_context(button);
  gtk_style_context_add_class(context, "control-button");
  gtk_style_context_add_class(context, "stop-button");
  gtk_box_pack_start(GTK_BOX(button_box), button, TRUE, TRUE, 0);
  g_signal_connect(button, "clicked", G_CALLBACK(stop_simulation), state);
  gtk_widget_set_sensitive(button, FALSE);

  button = gtk_button_new_with_label("Reset");
  context = gtk_widget_get_style_context(button);
  gtk_style_context_add_class(context, "control-button");
  gtk_style_context_add_class(context, "reset-button");
  gtk_box_pack_start(GTK_BOX(button_box), button, TRUE, TRUE, 0);
  g_signal_connect(button, "clicked", G_CALLBACK(reset_simulation), state);

  // Style the scheduler control card
  context = gtk_widget_get_style_context(frame);
  gtk_style_context_add_class(context, "dashboard-card");

  // Process Control Card
  frame = gtk_frame_new("Process Control");
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);
  gtk_box_pack_start(GTK_BOX(dashboard_box), frame, TRUE, TRUE, 0);

  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_container_add(GTK_CONTAINER(frame), box);

  // Add process button
  button = gtk_button_new_with_label("Add Process");
  context = gtk_widget_get_style_context(button);
  gtk_style_context_add_class(context, "control-button");
  gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 5);
  g_signal_connect(button, "clicked", G_CALLBACK(add_process), state);

  // Load process file button
  button = gtk_button_new_with_label("Load Process File");
  context = gtk_widget_get_style_context(button);
  gtk_style_context_add_class(context, "control-button");
  gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 5);
  g_signal_connect(button, "clicked", G_CALLBACK(load_process_file), state);

  // Step execution button
  button = gtk_button_new_with_label("Step Execution");
  context = gtk_widget_get_style_context(button);
  gtk_style_context_add_class(context, "control-button");
  gtk_style_context_add_class(context, "step-button");
  gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 5);
  g_signal_connect(button, "clicked", G_CALLBACK(step_simulation), state);

  // Auto execution button
  button = gtk_button_new_with_label("Auto Execute (5 steps)");
  context = gtk_widget_get_style_context(button);
  gtk_style_context_add_class(context, "control-button");
  gtk_style_context_add_class(context, "step-button");
  gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 5);
  g_signal_connect(button, "clicked", G_CALLBACK(auto_execution), state);
  state -> ui.auto_button = button;

  // Style the process control card
  context = gtk_widget_get_style_context(frame);
  gtk_style_context_add_class(context, "dashboard-card");

  // Middle Section: Process List and Queues
  GtkWidget * middle_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_box_pack_start(GTK_BOX(main_box), middle_box, TRUE, TRUE, 0);

  // Process List
  frame = gtk_frame_new("Process List");
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);
  gtk_box_pack_start(GTK_BOX(middle_box), frame, TRUE, TRUE, 0);

  scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add(GTK_CONTAINER(frame), scrolled_window);

  // Process list header
  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(scrolled_window), box);

  GtkWidget * header_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(box), header_box, FALSE, FALSE, 0);

  label = gtk_label_new("PID");
  gtk_widget_set_size_request(label, 40, -1);
  gtk_box_pack_start(GTK_BOX(header_box), label, FALSE, FALSE, 5);

  label = gtk_label_new("State");
  gtk_widget_set_size_request(label, 80, -1);
  gtk_box_pack_start(GTK_BOX(header_box), label, FALSE, FALSE, 5);

  label = gtk_label_new("Priority");
  gtk_widget_set_size_request(label, 60, -1);
  gtk_box_pack_start(GTK_BOX(header_box), label, FALSE, FALSE, 5);

  label = gtk_label_new("Memory");
  gtk_widget_set_size_request(label, 80, -1);
  gtk_box_pack_start(GTK_BOX(header_box), label, FALSE, FALSE, 5);

  label = gtk_label_new("PC");
  gtk_widget_set_size_request(label, 40, -1);
  gtk_box_pack_start(GTK_BOX(header_box), label, FALSE, FALSE, 5);

  // Process list content
  state -> ui.process_list = gtk_list_box_new();
  gtk_list_box_set_selection_mode(GTK_LIST_BOX(state -> ui.process_list), GTK_SELECTION_SINGLE);
  gtk_box_pack_start(GTK_BOX(box), state -> ui.process_list, TRUE, TRUE, 0);

  // Style the process list
  context = gtk_widget_get_style_context(frame);
  gtk_style_context_add_class(context, "dashboard-card");

  context = gtk_widget_get_style_context(scrolled_window);
  gtk_style_context_add_class(context, "scrolled");

  // Queues Section
  GtkWidget * queues_frame = gtk_frame_new("Queues");
  gtk_frame_set_shadow_type(GTK_FRAME(queues_frame), GTK_SHADOW_NONE);
  gtk_box_pack_start(GTK_BOX(middle_box), queues_frame, TRUE, TRUE, 0);

  GtkWidget * queues_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_container_add(GTK_CONTAINER(queues_frame), queues_box);

  // Running Process
  frame = gtk_frame_new("Running Process");
  gtk_box_pack_start(GTK_BOX(queues_box), frame, FALSE, TRUE, 0);

  state -> ui.running_process = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(frame), state -> ui.running_process);

  // Ready Queue
  frame = gtk_frame_new("Ready Queue");
  gtk_box_pack_start(GTK_BOX(queues_box), frame, TRUE, TRUE, 0);

  scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add(GTK_CONTAINER(frame), scrolled_window);

  state -> ui.ready_queue = gtk_list_box_new();
  gtk_container_add(GTK_CONTAINER(scrolled_window), state -> ui.ready_queue);

  // Blocked Queue
  frame = gtk_frame_new("Blocked Queue");
  gtk_box_pack_start(GTK_BOX(queues_box), frame, TRUE, TRUE, 0);

  scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add(GTK_CONTAINER(frame), scrolled_window);

  state -> ui.blocked_queue = gtk_list_box_new();
  gtk_container_add(GTK_CONTAINER(scrolled_window), state -> ui.blocked_queue);

  // Style the queues
  context = gtk_widget_get_style_context(queues_frame);
  gtk_style_context_add_class(context, "dashboard-card");

  // Resource & Memory Section (Bottom)
  resource_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_box_pack_start(GTK_BOX(main_box), resource_box, TRUE, TRUE, 0);

  // Memory Grid
  frame = gtk_frame_new("Memory (60 words)");
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);
  gtk_box_pack_start(GTK_BOX(resource_box), frame, TRUE, TRUE, 0);

  scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add(GTK_CONTAINER(frame), scrolled_window);

  state -> ui.memory_grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(state -> ui.memory_grid), 5);
  gtk_grid_set_column_spacing(GTK_GRID(state -> ui.memory_grid), 5);
  gtk_container_add(GTK_CONTAINER(scrolled_window), state -> ui.memory_grid);

  // Style the memory frame
  context = gtk_widget_get_style_context(frame);
  gtk_style_context_add_class(context, "dashboard-card");

  context = gtk_widget_get_style_context(scrolled_window);
  gtk_style_context_add_class(context, "scrolled");

  // Resource Mutexes
  frame = gtk_frame_new("Resource Mutexes");
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);
  gtk_box_pack_start(GTK_BOX(resource_box), frame, FALSE, TRUE, 0);

  state -> ui.mutex_status = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_container_add(GTK_CONTAINER(frame), state -> ui.mutex_status);

  // Style the mutex frame
  context = gtk_widget_get_style_context(frame);
  gtk_style_context_add_class(context, "dashboard-card");

  // Log Section
  log_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_box_pack_start(GTK_BOX(main_box), log_box, TRUE, TRUE, 0);

  // Execution Log
  frame = gtk_frame_new("Execution Log");
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);
  gtk_box_pack_start(GTK_BOX(log_box), frame, TRUE, TRUE, 0);

  scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add(GTK_CONTAINER(frame), scrolled_window);

  state -> ui.execution_log = gtk_list_box_new();
  gtk_container_add(GTK_CONTAINER(scrolled_window), state -> ui.execution_log);

  // Style the execution log
  context = gtk_widget_get_style_context(frame);
  gtk_style_context_add_class(context, "dashboard-card");

  context = gtk_widget_get_style_context(scrolled_window);
  gtk_style_context_add_class(context, "scrolled");

  // Event Messages
  frame = gtk_frame_new("Event Messages");
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);
  gtk_box_pack_start(GTK_BOX(log_box), frame, TRUE, TRUE, 0);

  scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add(GTK_CONTAINER(frame), scrolled_window);

  state -> ui.event_messages = gtk_list_box_new();
  gtk_container_add(GTK_CONTAINER(scrolled_window), state -> ui.event_messages);

  // Style the event messages
  context = gtk_widget_get_style_context(frame);
  gtk_style_context_add_class(context, "dashboard-card");

  context = gtk_widget_get_style_context(scrolled_window);
  gtk_style_context_add_class(context, "scrolled");

  // Show everything
  gtk_widget_show_all(window);

  // Initialize the UI components
  update_ui(state);

  // Add initial log entries
  add_log_entry(state -> ui.event_messages, "System initialized");
  add_log_entry(state -> ui.execution_log, "Ready to start simulation");

  return window;
}

// Scheduler implementation functions
static Process * select_next_process_fcfs(SimulationState * state) {
  if (state -> ready_queue == NULL) {
    return NULL;
  }

  // Return the first process in the ready queue (FIFO)
  Process * next_process = (Process * ) state -> ready_queue -> data;
  state -> ready_queue = g_list_remove(state -> ready_queue, next_process);

  return next_process;
}

static Process * select_next_process_round_robin(SimulationState * state) {
  if (state -> ready_queue == NULL) {
    return NULL;
  }

  // Return the first process in the ready queue and move it to the end
  Process * next_process = (Process * ) state -> ready_queue -> data;
  state -> ready_queue = g_list_remove(state -> ready_queue, next_process);

  return next_process;
}

static Process * select_next_process_mlfq(SimulationState * state) {
  if (state -> ready_queue == NULL) {
    return NULL;
  }

  // Simplified MLFQ: Find the process with highest priority (lowest number)
  Process * next_process = NULL;
  int highest_priority = 999;
  GList * highest_priority_node = NULL;

  for (GList * iter = state -> ready_queue; iter != NULL; iter = iter -> next) {
    Process * proc = (Process * ) iter -> data;
    if (proc -> priority < highest_priority) {
      highest_priority = proc -> priority;
      next_process = proc;
      highest_priority_node = iter;
    }
  }

  if (next_process) {
    state -> ready_queue = g_list_remove_link(state -> ready_queue, highest_priority_node);
    g_list_free_1(highest_priority_node);
  }

  return next_process;
}

static Process * schedule_next_process(SimulationState * state) {
  Process * next_process = NULL;

  // Select based on scheduling algorithm
  if (g_strcmp0(state -> current_algorithm, "FCFS") == 0) {
    next_process = select_next_process_fcfs(state);
  } else if (g_strcmp0(state -> current_algorithm, "Round Robin") == 0) {
    next_process = select_next_process_round_robin(state);
  } else if (g_strcmp0(state -> current_algorithm, "Multilevel Feedback") == 0) {
    next_process = select_next_process_mlfq(state);
  }

  if (next_process) {
    next_process -> state = RUNNING;

    char log_str[128];
    sprintf(log_str, "Scheduled process %d to run", next_process -> id);
    add_log_entry(state -> ui.event_messages, log_str);
  }

  return next_process;
}

static void increment_waiting_times(SimulationState * state) {
  // Increment time in queue for all processes in ready queue
  for (GList * iter = state -> ready_queue; iter != NULL; iter = iter -> next) {
    Process * proc = (Process * ) iter -> data;
    proc -> time_in_queue++;
  }

  // Increment time in queue for all processes in blocked queue
  for (GList * iter = state -> blocked_queue; iter != NULL; iter = iter -> next) {
    Process * proc = (Process * ) iter -> data;
    proc -> time_in_queue++;
  }
}

static void check_mutex_release(SimulationState * state) {
  // Randomly release resources with some probability
  if (state -> userInput_mutex && g_random_int_range(0, 10) > 7) {
    char log_str[128];
    sprintf(log_str, "Process %d released userInput mutex",
      state -> userInput_owner ? state -> userInput_owner -> id : -1);
    add_log_entry(state -> ui.event_messages, log_str);

    state -> userInput_mutex = FALSE;
    state -> userInput_owner = NULL;
  }

  if (state -> userOutput_mutex && g_random_int_range(0, 10) > 7) {
    char log_str[128];
    sprintf(log_str, "Process %d released userOutput mutex",
      state -> userOutput_owner ? state -> userOutput_owner -> id : -1);
    add_log_entry(state -> ui.event_messages, log_str);

    state -> userOutput_mutex = FALSE;
    state -> userOutput_owner = NULL;
  }

  if (state -> file_mutex && g_random_int_range(0, 10) > 7) {
    char log_str[128];
    sprintf(log_str, "Process %d released file mutex",
      state -> file_owner ? state -> file_owner -> id : -1);
    add_log_entry(state -> ui.event_messages, log_str);

    state -> file_mutex = FALSE;
    state -> file_owner = NULL;
  }
}

static void check_blocked_processes(SimulationState * state) {
  GList * still_blocked = NULL;

  // Check if any blocked process can be unblocked
  for (GList * iter = state -> blocked_queue; iter != NULL; iter = iter -> next) {
    Process * proc = (Process * ) iter -> data;
    gboolean can_unblock = FALSE;

    if (g_strcmp0(proc -> blocked_resource, "userInput") == 0 && !state -> userInput_mutex) {
      can_unblock = TRUE;
    } else if (g_strcmp0(proc -> blocked_resource, "userOutput") == 0 && !state -> userOutput_mutex) {
      can_unblock = TRUE;
    } else if (g_strcmp0(proc -> blocked_resource, "file") == 0 && !state -> file_mutex) {
      can_unblock = TRUE;
    }

    if (can_unblock) {
      char log_str[128];
      sprintf(log_str, "Process %d unblocked (resource %s available)",
        proc -> id, proc -> blocked_resource);
      add_log_entry(state -> ui.event_messages, log_str);

      // Move process to ready queue
      proc -> state = READY;
      proc -> time_in_queue = 0;
      g_free(proc -> blocked_resource);
      proc -> blocked_resource = NULL;
      state -> ready_queue = g_list_append(state -> ready_queue, proc);
    } else {
      still_blocked = g_list_append(still_blocked, proc);
    }
  }

  // Update blocked queue
  g_list_free(state -> blocked_queue);
  state -> blocked_queue = still_blocked;
}

static void execute_process_instruction(SimulationState * state) {
  Process * proc = state -> running_process;
  if (!proc) return;

  // Increment program counter
  proc -> program_counter++;

  char log_str[256];
  sprintf(log_str, "Cycle %d: Process %d executed instruction %s",
    state -> clock_cycle, proc -> id, proc -> instructions ? proc -> instructions : "N/A");
  add_log_entry(state -> ui.execution_log, log_str);

  // Simulate resource acquisition based on instruction
  if (proc -> instructions) {
    // Randomly simulate resource acquisition with some probability
    if (g_random_int_range(0, 10) > 6) {
      const char * resources[] = {
        "userInput",
        "userOutput",
        "file"
      };
      int resource_idx = g_random_int_range(0, 3);
      const char * resource = resources[resource_idx];
      gboolean blocked = FALSE;

      if (g_strcmp0(resource, "userInput") == 0) {
        if (state -> userInput_mutex) {
          blocked = TRUE;
        } else {
          state -> userInput_mutex = TRUE;
          state -> userInput_owner = proc;
          sprintf(log_str, "Process %d acquired userInput mutex", proc -> id);
          add_log_entry(state -> ui.event_messages, log_str);
        }
      } else if (g_strcmp0(resource, "userOutput") == 0) {
        if (state -> userOutput_mutex) {
          blocked = TRUE;
        } else {
          state -> userOutput_mutex = TRUE;
          state -> userOutput_owner = proc;
          sprintf(log_str, "Process %d acquired userOutput mutex", proc -> id);
          add_log_entry(state -> ui.event_messages, log_str);
        }
      } else if (g_strcmp0(resource, "file") == 0) {
        if (state -> file_mutex) {
          blocked = TRUE;
        } else {
          state -> file_mutex = TRUE;
          state -> file_owner = proc;
          sprintf(log_str, "Process %d acquired file mutex", proc -> id);
          add_log_entry(state -> ui.event_messages, log_str);
        }
      }

      if (blocked) {
        // Block the process and move to blocked queue
        proc -> state = BLOCKED;
        proc -> blocked_resource = g_strdup(resource);
        proc -> time_in_queue = 0;
        state -> blocked_queue = g_list_append(state -> blocked_queue, proc);
        state -> running_process = NULL;

        sprintf(log_str, "Process %d blocked waiting for %s", proc -> id, resource);
        add_log_entry(state -> ui.event_messages, log_str);
      }
    }

    // Generate a new instruction for simulation (random instruction)
    g_free(proc -> instructions);

    // List of possible instruction types for simulation
    const char * instr_types[] = {
      "COMPUTE",
      "READ",
      "WRITE",
      "INPUT",
      "OUTPUT",
      "ALLOCATE",
      "FREE"
    };
    int instr_idx = g_random_int_range(0, 7);

    // Generate a random operand (address or value)
    int operand = g_random_int_range(0, 100);

    char new_instr[64];
    sprintf(new_instr, "%s %d", instr_types[instr_idx], operand);
    proc -> instructions = g_strdup(new_instr);
  }

  // Check if time quantum expired for Round Robin
  if (g_strcmp0(state -> current_algorithm, "Round Robin") == 0) {
    static int time_slice = 0;
    time_slice++;

    if (time_slice >= state -> quantum && proc -> state == RUNNING) {
      time_slice = 0;

      // Preempt the process and put it back in ready queue
      sprintf(log_str, "Process %d preempted (quantum expired)", proc -> id);
      add_log_entry(state -> ui.event_messages, log_str);

      proc -> state = READY;
      proc -> time_in_queue = 0;
      state -> ready_queue = g_list_append(state -> ready_queue, proc);
      state -> running_process = NULL;
    }
  }
}

static void simulate_one_cycle(SimulationState * state) {
  // Increment clock cycle
  state -> clock_cycle++;

  // Check for mutex releases
  check_mutex_release(state);

  // Check blocked processes that might be unblocked
  check_blocked_processes(state);

  // Execute instruction for running process if any
  if (state -> running_process) {
    execute_process_instruction(state);
  }

  // Schedule a new process if none is running
  if (!state -> running_process && state -> ready_queue) {
    state -> running_process = schedule_next_process(state);
  }

  // Increment waiting times for processes in queues
  increment_waiting_times(state);

  // Update all UI components
  update_ui(state);
}

// Process file format parsing function
static gboolean parse_process_file(const char * filename, SimulationState * state) {
  // In a real implementation, this would parse the process file
  // For demonstration, we'll just add a mock process
  add_mock_process(state);
  return TRUE;
}

// Main function
int main(int argc, char * argv[]) {
    gtk_init(&argc, &argv);
    
    // Apply global CSS styles
    apply_global_css();
    
    // Initialize the simulation state
    SimulationState *state = init_simulation_state();
    
    // Build the UI
    GtkWidget *window = build_ui(state);
    
    // Add CSS to the main window
    GtkStyleContext *window_context = gtk_widget_get_style_context(window);
    gtk_style_context_add_class(window_context, "main-window");
    
    // Show the window
    gtk_widget_show_all(window);
    
    // Start the GTK main loop
    gtk_main();

  // Clean up
  for (GList * iter = state -> processes; iter != NULL; iter = iter -> next) {
    Process * proc = (Process * ) iter -> data;
    if (proc -> instructions) g_free(proc -> instructions);
    if (proc -> blocked_resource) g_free(proc -> blocked_resource);
    g_free(proc);
  }
  g_list_free(state -> processes);
  g_list_free(state -> ready_queue);
  g_list_free(state -> blocked_queue);
  if (state -> current_algorithm) g_free(state -> current_algorithm);
  g_free(state);

  return 0;
}
