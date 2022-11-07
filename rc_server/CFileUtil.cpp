/*
 * CFileUtil.cpp
 *
 *  Created on: 2015年4月14日
 *      Author: wq-test
 */

#include "CFileUtil.h"
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <algorithm>
#include <cstring>
#include <sys/vfs.h>
#include <iostream>
#include <sys/types.h>

static std::string const dir_sep("/");

struct FileInfo {
    bool is_link;
    off_t size;
};

bool get_env_var(std::string &value, std::string const &name) {
    char const *val
            = std::getenv(name.c_str());
    if (val)
        value = val;

    return val != nullptr;
}

struct is_empty_string
        : public std::unary_function<std::string const &, bool> {
    bool
    operator()(std::string const &str) const {
        return str.empty();
    }
};

template<typename Cont>
static
void
remove_empty(Cont &cont, std::size_t special) {
    cont.erase(
            std::remove_if(cont.begin() + special, cont.end(),
                           is_empty_string()),
            cont.end());
}

static
long
make_directory(std::string const &dir) {
    if (mkdir((dir).c_str(), 0777) == 0)

        return 0;
    else
        return errno;
}

static
std::string
get_current_dir() {
    std::string buf;
    std::string::size_type buf_size = 1024;
    char *ret;
    do {
        buf.resize(buf_size);
        ret = getcwd(&buf[0], buf.size());
        if (!ret) {
            int const eno = errno;
            if (eno == ERANGE)
                buf_size *= 2;
            else
                std::cout << "get_current_dir error " << std::endl;
        }
    } while (!ret);

    buf.resize(std::strlen(buf.c_str()));
    return buf;
}

struct path_sep_comp
        : public std::unary_function<char, bool> {
    bool
    operator()(char ch) const {
        return ch == '/';
    }
};

template<typename PathSepPred, typename Container>
static
void
split_into_components(Container &components, std::string const &path,
                      PathSepPred is_sep = PathSepPred()) {
    std::string::const_iterator const end = path.end();
    std::string::const_iterator it = path.begin();
    while (it != end) {
        std::string::const_iterator sep = std::find_if(it, end, is_sep);
        components.push_back(std::string(it, sep));
        it = sep;
        if (it != end)
            ++it;
    }
}

template<typename PathSepPred, typename Container>
static
void
expand_relative_path(Container &components,
                     PathSepPred is_sep = PathSepPred()) {
    // Get the current working director.

    std::string const cwd = get_current_dir();

    // Split the CWD.

    std::vector<std::string> cwd_components;

    // Use qualified call to appease IBM xlC.
    split_into_components(cwd_components, cwd, is_sep);

    // Insert the CWD components at the beginning of components.

    components.insert(components.begin(), cwd_components.begin(),
                      cwd_components.end());
}

bool
split_path(std::vector<std::string> &components, std::size_t &special,
           std::string const &path) {
    components.reserve(10);
    special = 0;

    // First split the path into individual components separated by
    // system specific separator.

    path_sep_comp is_sep;
    split_into_components(components, path, is_sep);

    // Try to recognize the path to find out how many initial components
    // of the path are special and should not be attempted to be created
    // using mkdir().

    retry_recognition:;
    std::size_t const comp_count = components.size();

    // "" "file", e.g., "/var/log/foo.0"
    if (comp_count >= 2
        && components[0].empty()) {
        remove_empty(components, 1);
        special = 1;
        return components.size() >= special + 1;
    }

        // "relative\path\to\some\file.log
    else {
        remove_empty(components, 0);
        expand_relative_path(components, is_sep);
        goto retry_recognition;
    }
}

template<typename Iterator>
inline
void
join(std::string &result, Iterator start, Iterator last, std::string const &sep) {
    if (start != last)
        result = *start++;

    for (; start != last; ++start) {
        result += sep;
        result += *start;
    }
}

int
getFileInfo(FileInfo *fi, std::string const &name) {
    struct stat fileStatus;
    if (stat((name).c_str(),
             &fileStatus) == -1)
        return -1;

    fi->is_link = S_ISLNK (fileStatus.st_mode);
    fi->size = fileStatus.st_size;

    return 0;
}
////////////////////////

CFileUtil::CFileUtil() = default;

CFileUtil::~CFileUtil() = default;

//false不匹配 true匹配 str1 入参  str2 匹配格式  例如  abcdfa,ab*f*
bool CFileUtil::strpp(const char *str1, const char *str2) {
    int i = 0;
    int flag = 0;
    if (str1 == nullptr || str2 == nullptr)
        return false;
    while (*str1 != '\0' && *str2 != '\0') {
        if (*str2 == '*') {
            str2++;
            i = 0;
            flag = 1;
            continue;
        }
        if (flag == 0) {
            if (*str1++ != *str2++)
                return false;
            else
                continue;
        }
        if (*str1 == *str2) {
            str1++;
            str2++;
            i++;
        } else {
            str1++;
            str2 -= i;
            i = 0;
        }
    }
    return (*str1 == '\0' && *str2 == '\0') || (*str2 == '\0' && *(str2 - 1) == '*');
}

string CFileUtil::getFileNameWithOutPostfix(string &path) {
    auto v = path.rfind('.');
    if (v == string::npos) {
        return path;
    }
    auto v0 = path.rfind('/');
    if (v0 == string::npos) {
        return path.substr(0, v);
    }
    return path.substr(v0 + 1, v - v0 - 1);
}

string CFileUtil::getFilePath(const string &path) {
    auto v0 = path.rfind('/');
    if (v0 == string::npos) {
        return "";
    }
    return path.substr(0, v0);
}

/*读取某给定路径下所有文件夹与文件名称，并带完整路径*/
void CFileUtil::getAllFile(const string &path, vector<string> &files, const char *matching, bool with_path) {
    DIR *dir = nullptr;
    struct dirent *res = nullptr;
    if ((dir = opendir(path.c_str())) == nullptr) {
        return;
    }

    while ((res = readdir(dir)) != nullptr) {
        if (res->d_type != DT_DIR)   //存放到列表中
        {
            if (matching != nullptr && !strpp(res->d_name, matching))
                continue;
            if (with_path) {
                files.push_back(std::move(path + "/" + res->d_name));
            } else {
                files.emplace_back(res->d_name);
            }
        }
    }
    closedir(dir); //关闭目录
}

int CFileUtil::lock_set(int fd, short type) {
    struct flock lock{};
    lock.l_type = type;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;
    lock.l_pid = -1;
    fcntl(fd, F_GETLK, &lock);
    if (lock.l_type != F_UNLCK) {
        if (lock.l_type == F_RDLCK)
            printf("Read lock already set by %d!\n", lock.l_pid);
        else if (lock.l_type == F_WRLCK)
            printf("Write lock already set by %d!\n", lock.l_pid);
    }
    lock.l_type = type;
    if (fcntl(fd, F_SETLKW, &lock) < 0) {
        printf("Lock failed:type=%d!\n", lock.l_type);
        return 1;
    }
    return 1;
}

string CFileUtil::getFileName(string path) {
    auto found = path.rfind('/');
    if (found == string::npos) {
        found = path.rfind('\\');
        if (found == string::npos) {
            return path;
        }
    }
    return path.substr(found + 1);
}

bool CFileUtil::touchFile(const string &filePath) {
    bool ret = false;
    make_dirs(filePath);
    ofstream of;
    of.open(filePath);
    if (of.is_open()) {
        ret = true;
    }
    of.close();
    return ret;
}

void CFileUtil::mvFile(vector<string> &files, const string &newPath) {
    auto its = files.begin();
    for (its = files.begin(); its != files.end(); ++its) {
        auto found = (*its).find_last_of("/\\");
        string strpath;
        if (found != string::npos)
            strpath = newPath + string("/") + (*its).substr(found);
        else
            strpath = newPath + string("/") + (*its);
        rename((*its).c_str(), strpath.c_str());
    }
}

void CFileUtil::mvFile(const string &oldPath, const string &newPath) {
    rename(oldPath.c_str(), newPath.c_str());
}

void CFileUtil::rmFile(vector<string> &files) {
    auto its = files.begin();
    for (its = files.begin(); its != files.end(); ++its) {
        remove((*its).c_str());
    }
}

int CFileUtil::make_dirs(std::string const &file_path) {
    std::vector<std::string> components;
    std::size_t special = 0;

    // Split file path into components.

    if (!split_path(components, special, file_path))
        return -1;

    // Remove file name from path components list.
    if (!(file_path.at(file_path.length() - 1) == '/' || file_path.at(file_path.length() - 1) == '\\')) {
        components.pop_back();
    }


    // Loop over path components, starting first non-special path component.

    std::string path;
    join(path, components.begin(), components.begin() + special,
         dir_sep);

    for (std::size_t i = special, components_size = components.size();
         i != components_size; ++i) {
        path += dir_sep;
        path += components[i];

        // Check whether path exists.

        FileInfo fi{};
        if (getFileInfo(&fi, path) == 0)
            // This directory exists. Move forward onto another path component.
            continue;

        // Make new directory.

        long const eno =
                make_directory(path);
        if (0 != eno) {
            return -1;
        }
    }
    return 0;
}

double CFileUtil::getDiskUsePercent(std::string const &file_path) {
    struct statfs diskInfo{};
    statfs(file_path.c_str(), &diskInfo);
    unsigned long long totalBlocks = diskInfo.f_bsize;
    unsigned long long totalSize = totalBlocks * diskInfo.f_blocks;
    unsigned long long freeDisk = diskInfo.f_bfree * totalBlocks;

    return (double) freeDisk / (double) totalSize;
}

double CFileUtil::getDiskUsePercent(std::string const &file_path, unsigned long long &totalSize,
                                    unsigned long long &freeDisk) {
    struct statfs diskInfo{};
    statfs(file_path.c_str(), &diskInfo);
    unsigned long long totalBlocks = diskInfo.f_bsize;
    totalSize = totalBlocks * diskInfo.f_blocks;
    freeDisk = diskInfo.f_bfree * totalBlocks;
    return (double) freeDisk / (double) totalSize;
}

string CFileUtil::getAllTextFromFile(const string &filePath) {
    std::ifstream t(filePath.c_str());
    if (!t) {
        return "";
    }
    std::string str((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());
    return str;
}

