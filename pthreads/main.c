#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pthread.h>


// 3. Scrieti un program 'copy' care se va lansa sub forma:
//      copy   f1 ... fn    f
//  (unde 'f1', ..., 'fn', 'f' sunt fisiere) si care are ca efect crearea sau
//  suprascrierea fisierului 'f' a.i. sa contina concatenarea fisierelor
//  'f1', ..., 'fn'; daca n = 1, se copiaza 'f1' in 'f'.
//    Programul isi va mapa cu 'mmap()' fisierul 'f' in memorie si va folosi
//  biblioteca 'Pthreads' pentru a genera n thread-uri paralele care vor copia
//  fiecare cate un fisier 'fi' in zona corespunzatoare din 'f'. In final, va
//  astepta terminarea thread-urilor si se va termina.


char *out_mmap;

typedef struct {
    const char *in_filename;
    off_t offset;
    off_t size;
} ThreadData;



void *concat_file_worker(void *arg) {
    ThreadData *data = (ThreadData *)arg;

    FILE* in_fd =fopen(data->in_filename, "r");
    if(in_fd == NULL) {
        perror("concat_file_worker: fopen");
        return NULL;
    }

    size_t bytes_to_read = (size_t)data->size;

    size_t bytes_read = fread(
        out_mmap + data->offset,
        1,
        bytes_to_read,
        in_fd
    );

    if(bytes_read != bytes_to_read) {
        if(ferror(in_fd)) {
            perror("concat_file_worker: fread");
        } else {
            fprintf(stderr, "Unexpected end of file\n"); // de verificat aici
        }
    }

    fclose(in_fd);
    return NULL;
}

int main(int argc, char *argv[]) {
    if(argc < 3) {
        printf("Utilizare: %s <f1_input> <f2_input> ... <fn_input> <f_output>\n Se accepta si %s <f1> <f_output>\n", argv[0], argv[0]);
        return 1;
    }

    const char *out_filename = argv[argc - 1];

    int num_in_files = argc - 2;

    const char *in_filenames[num_in_files];
    for(int i = 1; i <= num_in_files; i++) {
        in_filenames[i - 1] = argv[i];
    }

    // printf("debug: num_in_files = %d\n out_filename = %s\n", num_in_files, out_filename);
    // for(int i = 0; i < num_in_files; i++) {
    //     printf("debug: in_filenames[%d] = %s\n", i, in_filenames[i]);
    // }

    ThreadData t_data[num_in_files];

    off_t current_offset = 0;

    for(int i = 0; i < num_in_files; i++) {
        //https://man7.org/linux/man-pages/man2/stat.2.html
        struct stat st;
        if (stat(in_filenames[i], &st) == -1) {
            perror(in_filenames[i]);
            return 1;
        }

        if (!S_ISREG(st.st_mode)) { //din man inode, verificare fisier
            fprintf(stderr, "%s: not a regular file\n", in_filenames[i]);
            return 1;
        }

        t_data[i].in_filename = in_filenames[i];
        t_data[i].size = st.st_size;
        t_data[i].offset = current_offset;

        current_offset += st.st_size;
    }

    FILE *out_fd = fopen(out_filename, "w+");
    if (out_fd == NULL) { perror(out_filename); return 1; }
    int fd = fileno(out_fd);

    if (ftruncate(fd, current_offset) == -1) { perror("ftruncate"); return 1; }

    if (current_offset == 0) { fclose(out_fd); return 0; } // nimic de copiat

    //https://man7.org/linux/man-pages/man2/mmap.2.html
    out_mmap = mmap(NULL, current_offset, PROT_WRITE, MAP_SHARED, fd, 0);
    if (out_mmap == MAP_FAILED) { perror("mmap"); return 1; }
    fclose(out_fd);

    pthread_t threads[num_in_files];
    for(int i = 0; i < num_in_files; i++) {
        pthread_create(&threads[i], NULL, concat_file_worker, &t_data[i]);
    }

    for(int i = 0; i < num_in_files; i++) {
        pthread_join(threads[i], NULL);
    }

    msync(out_mmap, current_offset, MS_SYNC);
    munmap(out_mmap, current_offset);
    
    return 0;
}