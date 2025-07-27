#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>

#define BUFF_SIZE 1024
#define FILE_PATH "/dev/pseudo_char_device_"

void write_value(uint8_t file_number, char *value);
void read_value(uint8_t file_number);
void seek_value(uint8_t file_number, off_t offset, int whence);

int main(int argc, char *argv[])
{
    // printf("arg: %d, %s, %s, %s\n", argc, argv[0], argv[1], argv[2]);

    if ((argc < 2) || (argc > 5)) // total args- 3/4
    {
        printf("args = \
            [1] pcd file - [1-4]\n, \
            [2] operation to perform - [read - r / lseek - s /write - w] \n, \
            [3] value to write - [some_message]\n, \
            [3] lseek - position from the start\n");
        return EXIT_FAILURE;
    }
    if (atoi(argv[1]) < 1 || atoi(argv[1]) > 4)
    {
        printf("Invalid file number. Please enter a number between 1 and 4.\n");
        printf("pcd-1 - Read Only\n");
        printf("pcd-2 - Write Only\n");
        printf("pcd-3 - Read Write\n");
        printf("pcd-4 - Read Write\n");
        return EXIT_FAILURE;
    }

    switch (*argv[2])
    {
    case 'r':
    {
        if (argc != 3)
        {
            printf("Invalid arguments for read operation. Please provide only the file number and operation.\n");
            return EXIT_FAILURE;
        }

        read_value(atoi(argv[1]));
        break;
    }
    case 'w':
    {
        if (argc != 4)
        {

            printf("Invalid arguments for write operation. Please provide a value to write.\n");
            return EXIT_FAILURE;
        }

        write_value(atoi(argv[1]), argv[3]);
        break;
    }
    case 's':
    {
        if (argc != 4)
        {
            printf("Invalid arguments for seek operation. Please provide a position to seek to.\n");
            return EXIT_FAILURE;
        }
        if (atoi(argv[3]) < 0)
        {
            printf("Invalid seek position. Please enter a non-negative integer.\n");
            return EXIT_FAILURE;
        }

        seek_value(atoi(argv[1]), atoi(argv[3]), SEEK_SET);
        break;
    }
    default:
        printf("Invalid operation. Please use 'read', 'write', or 'seek'.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void write_value(uint8_t file_number, char *value)
{
    int fd, ret = 0;
    char buff[BUFF_SIZE];
    snprintf(buff, BUFF_SIZE, FILE_PATH "%d", file_number);

    fd = open(buff, O_WRONLY);
    if (fd < 0)
    {
        printf("Unable to open the file for Write: %s\n", buff);
        exit(EXIT_FAILURE);
    }
    // printf("PCD file-%d opened successfully = %d\n", file_number, fd);

    ret = write(fd, value, strlen(value));
    if (ret < 0)
    {
        printf("Unable to write into the file = %d\n", file_number);
        exit(EXIT_FAILURE);
    }
    printf("PCD file-%d write successfully\n", file_number);

    if (close(fd) < 0)
    {
        printf("Unable to close the file\n");
        exit(EXIT_FAILURE);
    }
    // printf("PCD file-%d closed successfully\n\n", file_number);
}

void read_value(uint8_t file_number)
{
    int fd, ret = 0;
    char buff[BUFF_SIZE];
    snprintf(buff, BUFF_SIZE, FILE_PATH "%d", file_number);

    fd = open(buff, O_RDONLY);
    if (fd < 0)
    {
        printf("Unable to open the file\n");
        exit(EXIT_FAILURE);
    }
    printf("PCD file-%d opened successfully for Read\n", file_number);

    memset(buff, 0, BUFF_SIZE);

    ret = read(fd, buff, BUFF_SIZE);
    if (ret < 0)
    {
        printf("Unable to read from the file = %d\n", file_number);
        exit(EXIT_FAILURE);
    }
    printf("PCD file-%d read successfully with Data: \n%s\n", file_number, buff);

    if (close(fd) < 0)
    {
        printf("Unable to close the file\n");
        exit(EXIT_FAILURE);
    }
    // printf("PCD file-%d closed successfully\n\n", file_number);
}

void seek_value(uint8_t file_number, off_t offset, int whence)
{
    int fd;
    char buff[BUFF_SIZE];
    snprintf(buff, BUFF_SIZE, FILE_PATH "%d", file_number);

    fd = open(buff, O_RDWR);
    if (fd < 0)
    {
        printf("Unable to open the file for Seek: %s\n", buff);
        exit(EXIT_FAILURE);
    }

    if (lseek(fd, offset, whence) < 0)
    {
        printf("Unable to seek in the file = %d\n", file_number);
        exit(EXIT_FAILURE);
    }
    // printf("PCD file-%d seek successfully\n", file_number);

    memset(buff, 0, BUFF_SIZE);

    if (read(fd, buff, BUFF_SIZE) < 0)
    {
        printf("Unable to read from the file = %d\n", file_number);
        exit(EXIT_FAILURE);
    }
    printf("PCD file-%d seek successfully with Data: \n %s\n", file_number, buff);

    if (close(fd) < 0)
    {
        printf("Unable to close the file\n");
        exit(EXIT_FAILURE);
    }
    // printf("PCD file-%d closed successfully\n\n", file_number);
}