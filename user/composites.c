#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include <stdarg.h>

int log_stdout(uint i) {
    /*
    Description: Redirect stdout to a log file named i.log.
    Example:
        - log_stdout(1); // Redirect the stdout to 1.log and return 0.
    Parameters:
        - i (uint): A number
    Return:
        - 0 (int)
    */
    char log_name[15] = "0.log";
    // Your code here
    uint base = 1, i_temp;
    if (i != 0) {
        for (base = 0, i_temp = i; i_temp != 0; ++base, i_temp /= 10);
        for (uint base_temp = 0, i_temp = i; i_temp != 0; ++base_temp, i_temp /= 10) {
            log_name[base - base_temp - 1] = '0' + i_temp % 10;
        }
        strcpy(log_name + base, ".log");
    }
    close(1);
    if (open(log_name, O_CREATE | O_WRONLY) != 1) {
        fprintf(2, "log_stdout: open failed\n");
        return -1;
    }
    // End
    return 0;
}

void sub_process(int p_left[2], int i) {
    /*
    Description:
        - Pseudocode:
            prime = get a number from left neighbor
            print prime m
            loop:
                m = get a number from left neighbor
                if (p does not divide m)
                    send m to right neighbor
                else
                    print composite m
        - Be careful to close file descriptors that a process doesn't need, 
        because otherwise your program will run xv6 out of resources before the first process reaches 35.
        - Hint: read returns zero when the write-side of a pipe is closed.
        - It's simplest to directly write 8-bit (1-byte) chars to the pipes, 
        rather than using formatted ASCII I/O.
        - Use pipe and fork to recursively set up and run the next sub_process 
        if necessary
        - Once the write-side of left neighbor is closed, 
        it should wait until the entire pipeline terminates, including all children, grandchildren, &c.
    Example:
        - sub_process(4); // Run the 4th sub_process.
    Parameters:
        - i (int): A number
    Return:
        - (void)
    */
     if (log_stdout(i) < 0) {
        fprintf(2, "composites: log_stdout %d failed\n", i);
        exit(1);
    }
    char m, prime;
    int num_read, p_right[2], pid = 0;
    // prime = get a number from left neighbor
    close(p_left[1]);
    num_read = read(p_left[0], &prime, 1);
    if (num_read == 0) {
        exit(0);
    }

    // End
    printf("prime %d\n", prime);
    while (1) {
        // m = get a number from left neighbor
        num_read = read(p_left[0], &m, 1);
        if (num_read == 0) {//如果读到的是0，说明左边的管道已经关闭了，所以退出
            exit(0);
        }
        // End
        // Use pipe and fork to recursively set up and run the next sub_process if necessary
        if (pid == 0) {//一个prime只能fork一次，所以只有pid==0的时候才能fork
            pipe(p_right);
            pid = fork();
            if (pid < 0) {
                fprintf(2, "composites: fork failed\n");
                exit(1);
            }
            else if (pid == 0) {
                sub_process(p_right, i + 1);
            }
        }

        // End
        if (m % prime != 0) {
            // send m to right neighbor
            close(p_right[0]);
            write(p_right[1], &m, 1);


            // End
        }
        else {
            printf("composite %d\n", m);
        }
    }
    // Once the write-side of left neighbor is closed, it should wait until the entire pipeline terminates, including all children, grandchildren, &c.
    close(p_left[0]);
    close(p_right[1]);
    while (wait(0) > 0);
    
    // End

    exit(0);
}

void composites() {
    /*
    Description:
        - A generating process can feed the numbers 2, 3, 4, ..., 35 into the left end of the pipeline: the first process in the line eliminates the multiples of 2, 
        the second eliminates the multiples of 3, the third eliminates the multiples of 5, and so on:
                +---------+    +---------+     +---------+     +---------+
            -2->| print 2 |    |         |     |         |     |         |
            -3->|         |-3->| print 3 |     |         |     |         |
            -4->| print 4 |    |         |     |         |     |         |
            -5->|         |-5->|         |- 5->| print 5 |     |         |
            -6->| print 6 |    |         |     |         |     |         |
            -7->|         |-7->|         |- 7->|         |- 7->| print 7 |
            -8->| print 8 |    |         |     |         |     |         |
            -9->|         |-9->| print 9 |     |         |     |         |
                +---------+    +---------+     +---------+     +---------+
        - Be careful to close file descriptors that a process doesn't need, because otherwise your program will run xv6 out of resources before the first process reaches 35.
        - Once the first process reaches 35, it should wait until the entire pipeline terminates, 
        including all children, grandchildren, &c. Thus the main composites process should only exit after all the output has been printed, and after all the other composites processes have exited.
        - You should create the processes in the pipeline only as they are needed.
    Example:
        - sub_process(4); // Run the 4th sub_process.
    Parameters:
    Return:
        - (void)
    */
    int p_right[2], pid, i = 0;
    // Use pipe and fork to recursively set up and run the first sub_process
    pipe(p_right);
    pid = fork();
    if(pid<0)
    {
        fprintf(2, "composites: fork failed\n");
        exit(1);
    }
    else if(pid==0)
    {
        sub_process(p_right, i);
    }
    
    // End
    // The first process feeds the numbers 2 through 35 into the pipeline.
    else
    {
        for (int j = 2; j <= 35; ++j) {
            close(p_right[0]);
            write(p_right[1], &j, 1);
        }
        close(p_right[1]);
    }
    // End
    // Once the first process reaches 35, it should wait until the entire pipeline terminates, 
    //including all children, grandchildren, &c. 
    //Thus the main primes process should only exit after all the output has been printed, 
    //and after all the other primes processes have exited.
    while(wait(0)>0);
    // End
    exit(0);
}

 int main(int argc, char* argv[])
 {
    if (argc != 1) {
        fprintf(2, "Usage: composites\n");
        exit(1);
    }
    composites();
    exit(0);
 }
