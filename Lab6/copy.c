#include <sys/types.h> 
#include <sys/stat.h> 
#include <sys/mman.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <stdbool.h>				
#include <stdio.h>					
#include <stdlib.h>					
#include <string.h>

// copy [-h] | [-m] <file_name> <new_file_name>  

int copy_read_write(int fd_from, int fd_to)
{
    char buffer[512]; // buffer for copied characters 
    ssize_t check; // check how many characters was read/written
    while (true)
    {
        check = read(fd_from, buffer, 512); // write 512 characters to buffer from source file
        if (check == -1) // if there was problem during reading process, leave
            return 1; 
        check = write(fd_to, buffer, check); // write 512 characters from buffer to dest. file
        if (check == -1) // if there was problem during writing process, leave   
            return 1;
        if (check == 0)  // if we write 0 characters to new file, it means met EOF and program can finish; break the infinite loop
            break;
    }
    return 0;
}
int copyMmap(int fd_from, int fd_to)
{
    off_t offset = lseek(fd_from, 0, SEEK_END); // check the size of source file 
    if (offset == -1) // if there was no possibility to read size, return
        return 2;
    lseek(fd_from, 0, SEEK_SET); // come back to the beginning of source file

    char *fd_from_map = mmap(0, offset, PROT_READ, MAP_PRIVATE, fd_from, 0); // map source file into memory 
    if (fd_from_map == MAP_FAILED)
        return 3;
    
    ftruncate(fd_to, offset); // ensure you can store the source file 
    char *fd_to_map = mmap(0, offset, PROT_WRITE, MAP_SHARED, fd_to, 0); //  map destination file into memory
    if (fd_to_map == MAP_FAILED)
        return 3;
    memcpy(fd_to_map, fd_from_map, offset); // copy memory
    
    munmap(fd_from_map, offset);  //unmap memory 
    munmap(fd_to_map, offset);
    
    return 0;
}
int main(int argc, char* argv[])
{
    int arg, fd_to, fd_from, error;
    bool mode_is_default = true;
    while ((arg = getopt(argc, argv, ":hm")) != -1) // using getopt check correctness of arguments; set proper mode if -m is present; display help if -h is present
    {
        switch (arg)
        {
        case 'h':
            printf("Usage:\n");
            printf("copy [-m] <file_name> <new_file_name>\n");
            printf("copy [-h]\n");
            printf("Note: [-m] mode for copying file use memory mapping, in deafult program copy using read() write() procedure\n");
            exit(0);
        case 'm':
            mode_is_default = false;
            break;
        default:
            fprintf(stderr, "Wrong usage, for more information type: copy -h\n");
            exit(1);
        }
    }
    
    if (argc - optind < 2) // if not enough arguments, display information about help command
    {
        fprintf(stderr, "Not enough arguments, for more information type: copy -h\n");
        exit(1);
    }
    
    fd_from = open(argv[optind], O_RDONLY); // open file as read only to copy from that file
    if (fd_from == -1) // if file does not exists or sth is broken, leave program
    {    
        fprintf(stderr, "Program cannot open source file or file does not exists!\n");
        exit(1);
    }
    fd_to = open(argv[optind+1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR); // open destination file or create one; open in read/write mode (read for mapping mode)
    if (fd_to == -1) // if cannot create file or open it, display error and leave program
    {    
        fprintf(stderr, "Program cannot open destination file or file cannot be created!\n");
        exit(1);
    }
    if (mode_is_default) // depending on user choice, either perform copy by read write or by memory mapping
        error = copy_read_write(fd_from,fd_to);
    else
        error = copyMmap(fd_from,fd_to);
    
    close(fd_from); // after copying close all files 
    close(fd_to);

    if (error == 0) // if copy was succesful or there was an error, display proper message
        printf("Files copied succesfully!\n");
    else if (error == 1)
        fprintf(stderr, "Unexpected error while reading/writing to file!\n");
    else if (error == 2)
        fprintf(stderr, "Program could not get the offset of file!\n");
    else if (error == 3)
        fprintf(stderr, "Program failed to map one of the files!\n");

    exit(error);
}