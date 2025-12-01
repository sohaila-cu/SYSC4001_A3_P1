/**
 * @file interrupts.cpp
 * @author Sasisekhar Govind 
 * @authors Sohaila Haroun & Zaineb BenHmida
 * @brief Scheduling simulator for using EP scheduling algorithm 
 */

#include<interrupts_101302936_101297624.hpp>

void FCFS(std::vector<PCB> &ready_queue) {
    std::sort( 
                ready_queue.begin(),
                ready_queue.end(),
                []( const PCB &first, const PCB &second ){
                    return (first.arrival_time > second.arrival_time); 
                } 
            );
}
//sort ready queue from highest priority (lowest PID) to lowest priority
void EP(std::vector<PCB> &ready_queue) {
    std::sort( 
                ready_queue.begin(),
                ready_queue.end(),
                []( const PCB &first, const PCB &second ){
                    return (first.PID > second.PID); 
                } 
            );
}
std::tuple<std::string /* add std::string for bonus mark */ > run_simulation(std::vector<PCB> list_processes) {

    std::vector<PCB> ready_queue;   //The ready queue of processes
    std::vector<PCB> wait_queue;    //The wait queue of processes
    std::vector<PCB> job_list;      //A list to keep track of all the processes. This is similar
                                    //to the "Process, Arrival time, Burst time" table that you
                                    //see in questions. You don't need to use it, I put it here
                                    //to make the code easier :).

    unsigned int current_time = 0;
    PCB running;

    //Initialize an empty running process
    idle_CPU(running);

    std::string execution_status;

    //make the output table (the header row)
    execution_status = print_exec_header();
    std::cout << "ok we in 1" << std::endl;
    //Loop while till there are no ready or waiting processes.
    //This is the main reason I have job_list, you don't have to use it.
    while(!all_process_terminated(job_list) || job_list.empty()) {
    std::cout << "ok we in 2" << std::endl;

        //Inside this loop, there are three things you must do:
        // 1) Populate the ready queue with processes as they arrive
        // 2) Manage the wait queue
        // 3) Schedule processes from the ready queue

        //Population of ready queue is given to you as an example.
        //Go through the list of proceeses
        for(auto &process : list_processes) {
            if(process.arrival_time == current_time ) {//check if the AT = current time
                //if so, assign memory and put the process into the ready queue
                if (assign_memory(process)){ //if memory allocation succeeded
                    process.state = READY;  //Set the process state to READY
                    ready_queue.push_back(process); //Add the process to the ready queue
                    job_list.push_back(process); //Add it to the list of processes
                    std::cout << "should be writing first line here" << std::endl;
                    execution_status += print_exec_status(current_time, process.PID, NEW, READY);
                }else{
                    process.state=NEW;
                    job_list.push_back(process);
                    sync_queue(list_processes,process);
                    std::cout<<"Error! Memory Allocation Failed"<<std::endl;
                }
            } 
            if (process.state==NEW && process.arrival_time < current_time) //it is not a new arrival but it is still waiting for memory
            {
                //try to assign memory and put the process into the ready queue
                if (assign_memory(process)){ //if memory allocation succeeded
                    process.state = READY;  //Set the process state to READY
                    ready_queue.push_back(process); //Add the process to the ready queue
                    job_list.push_back(process); //Add it to the list of processes
                    std::cout << "should be writing second line here" << std::endl;
                    execution_status += print_exec_status(current_time, process.PID, NEW, READY);
                }else{
                    std::cout<<"Error! Memory Allocation Failed Again"<<std::endl;
                }
            }
            
        }

        ///////////////////////MANAGE WAIT QUEUE/////////////////////////
        //This mainly involves keeping track of how long a process must remain in the ready (did you mean *wait) queue
        int wait_index = 0; //to keep track for removing
        for(auto &waiting : wait_queue) {
            //if this waiting process has finished doing IO, which is checked by:
            //calc if last time it started processing was an "IO-cycle" ago (where "IO-cycle"=freq+dur)
            if (waiting.start_time+waiting.io_freq+waiting.io_duration <= current_time){
                std::cout << "start-freq-dur = " << waiting.start_time+waiting.io_freq+waiting.io_duration  << std::endl;
                execution_status += print_exec_status(current_time, waiting.PID, WAITING, READY);
                waiting.state= READY;
                ready_queue.push_back(waiting);
                wait_queue.erase(wait_queue.begin()+wait_index);
                sync_queue(job_list, waiting);
            }
            else{ //only increment if we didn't remove cuz they shift
                wait_index++;
            }
        }

        /////////////////////////////////////////////////////////////////

        //////////////////////////SCHEDULER//////////////////////////////
        std::cout << "ok we in 4" << std::endl;
        if (running.state==RUNNING&& running.remaining_time == 0) //higher priority than I/O
        {
                    std::cout << "ok we in 6" << std::endl;

            terminate_process(running,job_list);
            execution_status += print_exec_status(current_time, running.PID, RUNNING, TERMINATED);
            sync_queue(job_list,running);
            idle_CPU(running);
            //sync_queue(job_list, running);
        }
        else if ((current_time-running.start_time)== running.io_freq && running.io_freq!=0) //if time to do I/O
        {
                    std::cout << "ok we in 5" << std::endl;

            running.state = WAITING;
            //running.remaining_time-=running.io_freq; //not sure abt this yet, nvm remove
            execution_status += print_exec_status(current_time, running.PID, RUNNING, WAITING);
            wait_queue.push_back(running);
            sync_queue(job_list, running); //only really important for termination
            idle_CPU(running);
            //sync_queue(job_list, running);
        }


        if (running.state == NOT_ASSIGNED && !ready_queue.empty()){ //if cpu idle
                    std::cout << "ok we in 7" << std::endl;

            EP(ready_queue); //sort ready queue
            run_process(running,job_list,ready_queue,current_time); //run next process
            execution_status += print_exec_status(current_time, running.PID, READY, RUNNING);
        }
        

        
        // std::cout << "ok we in 5" << std::endl;
        // if (!all_process_terminated(job_list)){
        //     std::cout << "I'm but not terminated?" << std::endl;
        // }
        // if (job_list.empty()){
        //     std::cout << "I'm emptyyyyyyyyyyyy" << std::endl;
        //     break;
        // }

        /////////////////////////////////////////////////////////////////
        if (running.state==RUNNING)
        {
            running.remaining_time-=1; //decrement remaining process time if running
        }
        
        current_time++; //increment time once per loop
    }
    
    //Close the output table
    execution_status += print_exec_footer();
    std::cout << "ok we out" << std::endl;

    return std::make_tuple(execution_status);
}


int main(int argc, char** argv) {
    std::cout << "Hello" << std::endl;
    //Get the input file from the user
    if(argc != 2) {
        std::cout << "ERROR!\nExpected 1 argument, received " << argc - 1 << std::endl;
        std::cout << "To run the program, do: ./interrupts <your_input_file.txt>" << std::endl;
        return -1;
    }
    //Open the input file
    auto file_name = argv[1];
    std::ifstream input_file;
    input_file.open(file_name);
    std::cout << "1" << std::endl;
    //Ensure that the file actually opens
    if (!input_file.is_open()) {
        std::cerr << "Error: Unable to open file: " << file_name << std::endl;
        return -1;
    }
    std::cout << "2" << std::endl;

    //Parse the entire input file and populate a vector of PCBs.
    //To do so, the add_process() helper function is used (see include file).
    std::string line;
    std::vector<PCB> list_process;
    while(std::getline(input_file, line)) {
        auto input_tokens = split_delim(line, ", ");
        auto new_process = add_process(input_tokens);
        list_process.push_back(new_process);
    }
    input_file.close();
    std::cout << "3" << std::endl;

    //With the list of processes, run the simulation
    auto [exec] = run_simulation(list_process);
    std::cout << "4" << std::endl;

    write_output(exec, "execution.txt");
    std::cout << "5" << std::endl;

    return 0;
}