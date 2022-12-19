/**
 * @brief Driver for FAT file system on the SD card.
 */

#ifndef _FILE_SYSTEM_H_
#define _FILE_SYSTEM_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

namespace file_system {
    /**
     * @brief Initialises the SD controller and mounts the FAT file system on
     * it.
     *
     * @return false if error occured or if SD card is not present.
     */
    bool initialise();

    /**
     * @breif Unmounts the FAT file system on the SD card and de-initialises the
     * SD controller.
     *
     * @return true if unmount was successful.
     */
    bool deinitialise();

    /**
     * @brief Sets the current directory.
     *
     * @return True if current directory was set successfully.
     */
    bool cd(const char* path);

    /**
     * @brief Retrieves the file names in a directory.
     *
     * @note This function will allocate the file names buffer returned on the
     * heap, and has to be freed manually. Furthermore, the file names will not
     * be in sequential order sorted after name.
     *
     * @param out_number_of_files The number of files in the directory.
     * @param out_file_names Pointer to the array of the name of the files. NULL
     * if no files were found.
     *
     * @return true if files were found in the directory.
     */
    bool ls(char*** out_file_names, size_t* out_number_of_files);

    /**
     * @brief Retrieves the file size of a file at the given @p path.
     *
     * @param path The path to the file.
     * @param out_file_size_ptr Pointer to the value where the file size will be
     * placed.
     *
     * @return True if the file exists and the file size was retrieved.
     */
    bool size(const char* path, uint32_t* out_file_size_ptr);

    /**
     * @brief Reads @p bytes_to_read from the file at @p path into the @p
     * buffer.
     *
     * @param path Path to the file to be read.
     * @param out_buffer Where the data is placed.
     * @param bytes_to_read Amount of bytes to read from the file. Can be less
     * than the file size.
     *
     * @return true if read was successful.
     */
    bool
    read(const char* path, uint8_t* out_buffer, const uint32_t bytes_to_read);

    /**
     * @brief Writes a @p buffer into a file at @p path.
     *
     * @param path Path to the file.
     * @param buffer The content to write.
     * @param bytes_to_read The length of the buffer has to be at least equal to
     * or greater in length than the amount of bytes to read.
     */
    bool write(const char* path,
               const uint8_t* buffer,
               const uint32_t buffer_length);

}

#endif
