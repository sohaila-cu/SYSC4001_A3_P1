
/**
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 * @authors Sohaila Haroun & Zaineb BenHmida
 * @brief Scheduling simulator for using EP&RR scheduling algorithms, where the preemption is from just RR
 * 
 */

#include"interrupts_101302936_101297624.hpp"

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

std::tuple<std::string, std::string /* add std::string for bonus mark */ > run_simulation(std::vector<PCB> list_processes) {
    std::vector<PCB> ready_queue;   //The ready queue of processes
    std::vector<PCB> wait_queue;    //The wait queue of processes
    std::vector<PCB> job_list;      //A list to keep track of all the processes. This is similar
                                    //to the "Process, Arrival time, Burst time" table that you
                                    //see in questions. You don't need to use it, I put it here
                                    //to make the code easier :).

    unsigned int current_time = 0;
    unsigned int total_wait_time=0;
    unsigned int turnaround_time=0;
    PCB running;

    //Initialize an empty running process
    idle_CPU(running);

    std::string execution_status;
    std::string memory_status;

    //make the output table (the header row)
    execution_status = print_exec_header();

    //Loop while till there are no ready or waiting processes.
    //This is the main reason I have job_list, you don't have to use it.
    while(!all_process_terminated(job_list) || job_list.empty()) {
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
                    execution_status += print_exec_status(current_time, process.PID, NEW, READY);
                    sync_queue(job_list,running);
                    memory_status += print_memory_status(current_time, job_list);
                }else{
                    process.state=NEW;
                    job_list.push_back(process);
                    sync_queue(list_processes,process);
                    memory_status += print_memory_status(current_time, job_list);
                    memory_status += "Memory Allocation Failure Occured, Process will wait until memory is available.\n\n";
                }
            } 
            if (process.state==NEW && process.arrival_time < current_time) //it is not a new arrival but it is still waiting for memory
            {
                //try to assign memory and put the process into the ready queue
                if (assign_memory(process)){ //if memory allocation succeeded
                    process.state = READY;  //Set the process state to READY
                    ready_queue.push_back(process); //Add the process to the ready queue
                    job_list.push_back(process); //Add it to the list of processes
                    execution_status += print_exec_status(current_time, process.PID, NEW, READY);
                    memory_status += "Previous Memory Allocation Failure Resolved, memory was available.";  
                    memory_status += print_memory_status(current_time, job_list);              

                }else{
                    //too redundant
                    //memory_status += print_memory_status(current_time, job_list);
                    //memory_status += "Memory Re-Allocation Failure Occured, Process will continue waiting.";
                }
            } 
        }

        ///////////////////////MANAGE WAIT QUEUE/////////////////////////
        //This mainly involves keeping track of how long a process must remain in the ready queue
        //This mainly involves keeping track of how long a process must remain in the ready (did you mean *wait) queue
        int wait_index = 0; //to keep track for removing
        for(auto &waiting : wait_queue) {
            //if this waiting process has finished doing IO, which is checked by:
            //calc if last time it started processing was an "IO-cycle" ago (where "IO-cycle"=freq+dur)
            if (waiting.start_time+waiting.io_freq+waiting.io_duration <= current_time){
                execution_status += print_exec_status(current_time, waiting.PID, WAITING, READY);
                waiting.state= READY;
                ready_queue.push_back(waiting);
                wait_queue.erase(wait_queue.begin()+wait_index);

            }
            else{ //only increment if we didn't remove cuz they shift
                wait_index++;
            }
        }
        /////////////////////////////////////////////////////////////////

        //////////////////////////SCHEDULER//////////////////////////////

        if (running.state==RUNNING){
            if (running.state==RUNNING&& running.remaining_time == 0) //higher priority than I/O
            {
                turnaround_time += (current_time-running.arrival_time); //add to total
                if (running.io_freq!=0){
                    total_wait_time += (current_time-running.arrival_time-running.processing_time-(running.io_duration*(running.processing_time/running.io_freq)));
                }
                else
                {
                    total_wait_time += (current_time-running.arrival_time-running.processing_time);
                }
                terminate_process(running,job_list);
                execution_status += print_exec_status(current_time, running.PID, RUNNING, TERMINATED);
                sync_queue(job_list,running);
                idle_CPU(running);
                //sync_queue(job_list, running);
            }
            else if (running.io_freq!=0 &&((running.processing_time-running.remaining_time)% running.io_freq) ==0) //if time to do I/O 
            {
                running.state = WAITING;
                //running.remaining_time-=running.io_freq; //not sure abt this yet, nvm remove
                execution_status += print_exec_status(current_time, running.PID, RUNNING, WAITING);
                wait_queue.push_back(running);
                sync_queue(job_list, running); //only really important for termination
                idle_CPU(running);
                //sync_queue(job_list, running);
            }
            else if (running.state==RUNNING&&((current_time-running.start_time)>99))
            {
                execution_status+=print_exec_status(current_time, running.PID, RUNNING, READY);
                running.state=READY;
                ready_queue.push_back(running);
                idle_CPU(running);
                //sync_queue(job_list, running);
            }
        }

        if (running.state == NOT_ASSIGNED && !ready_queue.empty()){ //if cpu idle
            EP(ready_queue); 
            run_process(running,job_list,ready_queue,current_time); //run next process
            execution_status += print_exec_status(current_time, running.PID, READY, RUNNING);
        }


        /////////////////////////////////////////////////////////////////
        if (running.state==RUNNING)
        {
            running.remaining_time-=1; //decrement remaining process time if running
        }
        
        current_time++; //increment time once per loop
    }
    //display metrics (will be recorded possibly, didn't specifiy in assignment)
    std::cout<<"Total Turnaround Time: "<< turnaround_time<<"ms"<<std::endl;
    std::cout<<"Total Wait Time: "<< total_wait_time<<"ms"<<std::endl;

    std::cout<<"Average Turnaround Time: "<< turnaround_time/job_list.size()<<"ms/job"<<std::endl;
    std::cout<<"Average Wait Time: "<< total_wait_time/job_list.size()<<"ms/job"<<std::endl;
    std::cout<<"Throughput: "<< job_list.size()<<"/"<<current_time<<"ms"<<std::endl;
    std::cout<<"Average Response Time: "<<std::endl;

    //Close the output table
    execution_status += print_exec_footer();

    memory_status += print_memory_status(current_time, job_list);              
    return std::make_tuple(execution_status,memory_status);}


int main(int argc, char** argv) {
    //Get the input file from the user
    if(argc != 2) {
        std::cout << "ERROR!\nExpected 1 argument, received " << argc - 1 << std::endl;
        std::cout << "To run the program, do: ./interrutps <your_input_file.txt>" << std::endl;
        return -1;
    }
    //Open the input file
    auto file_name = argv[1];
    std::ifstream input_file;
    input_file.open(file_name);

    //Ensure that the file actually opens
    if (!input_file.is_open()) {
        std::cerr << "Error: Unable to open file: " << file_name << std::endl;
        return -1;
    }

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

    //With the list of processes, run the simulation
    auto [exec, mem] = run_simulation(list_process);
    write_output(exec, "execution.txt");
    write_output(mem, "memory.txt");

    return 0;
}
