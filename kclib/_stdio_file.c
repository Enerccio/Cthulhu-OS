#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "intinc/stdio.h"

FILE __stderr;
FILE*  stderr;
FILE __stdout;
FILE*  stdout;
FILE __stdin;
FILE*  stdin;

int fclose(FILE* stream){
	if (!__IS_CLOSEABLE(stream->fflags))
		return 0;

	int err = fflush(stream);
	__free_buffer(&stream->buffer);
	if (__IS_HASBUFFER(stream->fflags)){
		for (size_t i=0; i<__buffered_handles_len; i++){
			if (__buffered_handles[i] == stream)
				__buffered_handles[i] = NULL;
		}
	}
	return err;
}

int fflush(FILE* stream){
	if (stream == NULL){
		int retv = 0;
		for (size_t i=0; i<__buffered_handles_len; i++){
			if (__buffered_handles[i] != NULL)
				retv |= fflush(__buffered_handles[i]);
		}
		return retv;
	}

	if (stream->virtual)
		return 0; // virtual stream does not do anything except operating with buffer

	if (__IS_CLOSEABLE(stream->fflags) && stream->closed)
		return EOF;
	if (!__IS_HASBUFFER(stream->fflags)){
		return 0;
	}

	if (stream->lastwrite){
		size_t len;
		uint8_t* data = __buffer_get_data(&stream->buffer, &len);

		ptrdiff_t datasent = __kclib_send_data(stream->handle, data, len);

		__buffer_ftell(&stream->buffer, 0);
		stream->lastwrite = false;

		if (datasent < 0){
			stream->error = __FERROR_WRITE;
			return EOF;
		} else {
			if ((size_t)datasent < len){
				return EOF;
			} else {
				return 0;
			}
		}
	} else {
		stream->lastwrite = true;
	}
	return 0;
}

FILE* __create_filehandle(void* pd){
	FILE* f = malloc(sizeof(FILE));
	if (f == NULL)
		return NULL;
	memset(f, 0, sizeof(FILE));
	f->handle = pd;
	return f;
}

void __init_std_stream(FILE* stream, void* fd, int mode){
	stream->handle = fd;
	setvbuf(stream, NULL, mode, BUFSIZ);
}

void __initialize_streams(){
	__buffered_handles = 0;
	__buffered_handles_len = 0;

	stdout = &__stdout;
	stderr = &__stderr;
	stdin  = &__stdin;

	memset(stdout, 0, sizeof(FILE));
	memset(stderr, 0, sizeof(FILE));
	memset(stdin, 0, sizeof(FILE));

	__init_std_stream(stdout, __kclib_open_std_stream(__STDOUT), _IOLBF);
	__init_std_stream(stderr, __kclib_open_std_stream(__STDERR), _IOLBF);
	__init_std_stream(stdin, __kclib_open_std_stream(__STDIN), _IONBF);
}

FILE* fopen(const char* restrict filename,
		const char * restrict mode){
#ifdef __KCLIB_KERNEL_MODE
	return NULL; // no support in kernel mode
#else
	FILE* fp = malloc(sizeof(FILE));
	if (fp == NULL)
		return NULL;

	int mmode = 0;
	size_t modelen = strlen(mode);
	if (modelen == 3){
		if (strncmp(mode, "a+b", 3) == 0 || strncmp(mode, "ab+", 3) == 0){
			mmode = __MODE_APPEND | __MODE_BINARY | __MODE_UPDATE;
		}

		if (strncmp(mode, "w+b", 3) == 0 || strncmp(mode, "wb+", 3) == 0){
			mmode = __MODE_WRITE | __MODE_BINARY | __MODE_UPDATE;
		}

		if (strncmp(mode, "r+b", 3) == 0 || strncmp(mode, "rb+", 3) == 0){
			mmode = __MODE_READ | __MODE_BINARY | __MODE_UPDATE;
		}
	} else if (modelen == 2){
		if (strncmp(mode, "r+", 2) == 0){
			mmode = __MODE_READ | __MODE_UPDATE;
		}
		if (strncmp(mode, "w+", 2) == 0){
			mmode = __MODE_WRITE | __MODE_UPDATE;
		}
		if (strncmp(mode, "a+", 2) == 0){
			mmode = __MODE_APPEND | __MODE_UPDATE;
		}
		if (strncmp(mode, "rb", 2) == 0){
			mmode = __MODE_READ | __MODE_BINARY;
		}
		if (strncmp(mode, "wb", 2) == 0){
			mmode = __MODE_WRITE | __MODE_BINARY;
		}
		if (strncmp(mode, "ab", 2) == 0){
			mmode = __MODE_APPEND | __MODE_BINARY;
		}
	} else if (modelen == 1){
		if (strncmp(mode, "r", 1) == 0){
			mmode = __MODE_READ;
		}
		if (strncmp(mode, "w", 1) == 0){
			mmode = __MODE_WRITE;
		}
		if (strncmp(mode, "a", 1) == 0){
			mmode = __MODE_APPEND;
		}
	}

	if (mmode == 0) {
		free(fp);
		return NULL;
	}

	memset(fp, 0, sizeof(FILE));
	fp->handle = __kclib_open_file_u(filename, mmode);

	if (fp->handle == NULL){
		free(fp);
		return NULL;
	}

	fp->stat = malloc(sizeof(stat_t));
	if (fp->stat == NULL){
		free(fp);
		return NULL;
	}

	if (__kclib_fstat_u(fp->handle, fp->stat, &errno)){
		free(fp->stat);
		free(fp);
		return NULL;
	}

	if (fp->stat != NULL && fp->stat->st_mode == S_IFBLK){
		if (setbuf(fp, NULL)){ // default buffer
			free(fp->stat);
			free(fp);
		}
	}

	return fp;
#endif
}

size_t fread(void* restrict ptr,
		size_t size, size_t nmemb,
		FILE* restrict stream){
	if (size == 0 || nmemb == 0)
		return 0;

	size_t readcount = size * nmemb;
	uint8_t* resbuf = (uint8_t*)ptr;

	if (!__IS_HASBUFFER(stream->fflags)){
		// no buffer, read directly from stream
		ptrdiff_t ra =  __kclib_read_data(stream->handle, resbuf, readcount);
		if (ra < 0){
			stream->error = __FERROR_READ;
			return 0;
		} else
			return ra;
	} else {
		if (stream->lastwrite){
			__buffer_ftell(&stream->buffer, 0);
		}

		size_t readc = 0;
		do {
			size_t rc = __read_from_buffer(&stream->buffer, resbuf, readcount);
			resbuf += rc;
			readcount -= rc;
			readc += rc;

			if (__buffer_usedsize(&stream->buffer) != 0){
				return readc;
			}

			if (!stream->virtual){
				ptrdiff_t osra = __kclib_read_data(stream->handle, stream->buffer.buffer, stream->buffer.limit);
				if (osra < 0){
					stream->error = __FERROR_READ;
					return readc;
				}
				stream->buffer.cpos = (size_t)osra;
			}


			if (readcount == 0)
				return readc;
			else {
				if (__buffer_freesize(&stream->buffer) == __buffer_maxsize(&stream->buffer)){
					return readc;
				}
			}
		} while (true);
	}
}

size_t fwrite(const void* restrict ptr,
		size_t size, size_t nmemb,
		FILE* restrict stream){
	if (size == 0 || nmemb == 0)
		return 0;

	size_t writecount = size * nmemb;
	uint8_t* writebuf = (uint8_t*)ptr;

	if (!__IS_HASBUFFER(stream->fflags)){
		// no buffer, read directly from stream
		ptrdiff_t ra =  __kclib_send_data(stream->handle, writebuf, writecount);
		if (ra < 0){
			stream->error = __FERROR_READ;
			return 0;
		} else
			return ra;
	} else {
		if (!stream->lastwrite){
			if (fflush(stream) != 0)
				return 0;
		}

		size_t writec = 0;
		do {
			if (!stream->virtual && __buffer_freesize(&stream->buffer)<writecount){
				ptrdiff_t oswa = __kclib_send_data(stream->handle, stream->buffer.buffer, stream->buffer.cpos);
				if (oswa < 0){
					stream->error = __FERROR_WRITE;
					return writec;
				} else if (oswa == 0 && __buffer_freesize(&stream->buffer) == 0){
					stream->error = __FERROR_BUFFULL;
					return writec;
				}
				__buffer_shift(&stream->buffer, (size_t)oswa);
			}

			bool flushit;
			size_t wc = __write_to_buffer(&stream->buffer, writebuf, writecount, &flushit);
			writebuf += wc;
			writecount -= wc;
			writec += wc;

			if (flushit){
				fflush(stream);
			}

			if (writecount == 0)
				return writec/size;
		} while (true);
	}
}

FILE* __create_vstream(uint8_t* backing_array, size_t n, uint8_t mode){
	FILE* vf = malloc(sizeof(FILE));
	if (vf == NULL)
		return NULL;

	memset(vf, 0, sizeof(FILE));
	vf->virtual = true;
	if (setvbuf(vf, (char*)backing_array, mode, n)){
		free(vf);
		return NULL;
	}

	return vf;
}

int fseek(FILE *stream, long int offset, int whence){
	return 0;
}

long int ftell(FILE *stream){
	return 0;
}
