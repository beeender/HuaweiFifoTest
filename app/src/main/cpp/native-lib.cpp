#include <jni.h>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sstream>
#include <errno.h>

bool try_open(const char* path, std::ostringstream& output_stream) {
    output_stream << "Test to open " << path << std::endl;

    struct stat stat_buf;
    if (stat(path, &stat_buf) == 0) {
        output_stream << "Its uid is :" << stat_buf.st_uid << std::endl;
    } else {
        output_stream << "failed to stat()" << std::endl;
    }

    int fd = open(path, O_RDWR);
    if (fd == -1) {
        int error = errno;
        output_stream << "failed with error:" << error << std::endl;
        return false;
    } else {
        output_stream << "Passed" << std::endl;
        close(fd);
    }
    return true;
}

extern "C"
JNIEXPORT jstring

JNICALL
Java_tt_test_huaweififotest_MainActivity_fifoTest(
        JNIEnv *env,
        jobject /* this */,
        jstring j_files_dir) {

    std::ostringstream output_stream;
    output_stream << "Current process's uid is: " << getuid() << std::endl;
    output_stream << std::endl;

    const char* files_dir = env->GetStringUTFChars(j_files_dir, nullptr);

    std::string normal_file_path(files_dir);
    normal_file_path += "/normal_file";

    output_stream << "Test to create normal file, path: '" << normal_file_path << "'" << std::endl;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int ret = open(normal_file_path.c_str(), O_RDWR | O_CREAT | O_EXCL, mode);
    if (ret != -1) {
        output_stream << "succeed, created a new normal file." << std::endl;
        ret = close(ret);
    } else {
        int error = errno;
        if (error == EEXIST) {
            output_stream << "succeed, file exists." << std::endl;
        } else {
            output_stream << "failed to create normal file";
        }
    }
    try_open(normal_file_path.c_str(), output_stream);

    output_stream << std::endl;

    std::string fifo_path(files_dir);
    fifo_path += "/fifo";

    output_stream << "Test to create mkfifo, path: '" << fifo_path  << "'" << std::endl;
    ret = mkfifo(fifo_path.c_str(), 0600);
    if (ret == 0) {
        output_stream << "succeed, created a new fifo." << std::endl;
    } else {
        int error = errno;
        if (error == EEXIST) {
            output_stream << "succeed, fifo exists." << std::endl;
        } else {
            output_stream << "failed";
            return env->NewStringUTF(output_stream.str().c_str());
        }
    }

    try_open(fifo_path.c_str(), output_stream);

    return env->NewStringUTF(output_stream.str().c_str());
}
