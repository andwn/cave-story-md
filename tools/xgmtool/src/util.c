#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "../inc/util.h"

LList* createEmptyElement()
{
    LList* result = malloc(sizeof(LList));

    result->element = NULL;
    result->next = NULL;
    result->prev = NULL;

    return result;
}

LList* createElement(void* element)
{
    LList* result = malloc(sizeof(LList));

    result->element = element;
    result->next = NULL;
    result->prev = NULL;

    return result;
}

void deleteLList(LList* list)
{
    LList* l = list;

    while(l != NULL)
    {
        LList* n = l->next;
        free(l);
        l = n;
    }
}

static void connectNext(LList* element, LList* next)
{
    if (element != NULL) element->next = next;
    if (next != NULL) next->prev = element;
}

static void connectPrev(LList* element, LList* prev)
{
    if (element != NULL) element->prev = prev;
    if (prev != NULL) prev->next = element;
}

LList* getHeadLList(LList* list)
{
    LList* result = list;

    if (result == NULL) return NULL;

    while(result->prev != NULL) result = result->prev;

    return result;
}

LList* getTailLList(LList* list)
{
    LList* result = list;

    if (result == NULL) return NULL;

    while(result->next != NULL) result = result->next;

    return result;
}

int getSizeLList(LList* list)
{
    int result = 0;
    LList* l = list;

    while(l)
    {
        l = l->next;
        result++;
    }

    return result;
}

LList* getElementAtLList(LList* list, int index)
{
    int i = 0;
    LList* l = list;

    while(l)
    {
        if (i == index) return l;

        l = l->next;
        i++;
    }

    return NULL;
}

LList* insertAfterLList(LList* linkedElement, void* element)
{
    LList* newElement = createElement(element);

    if (linkedElement != NULL)
    {
        connectNext(newElement, linkedElement->next);
        connectPrev(newElement, linkedElement);
    }

    // return inserted element
    return newElement;
}

LList* insertBeforeLList(LList* linkedElement, void* element)
{
    LList* newElement = createElement(element);

    if (linkedElement != NULL)
    {
        connectPrev(newElement, linkedElement->prev);
        connectNext(newElement, linkedElement);
    }

    // return inserted element
    return newElement;
}

LList* insertAllAfterLList(LList* linkedElement, LList* elements)
{
    LList* src = elements;
    LList* dst = linkedElement;

    while(src != NULL)
    {
        dst = insertAfterLList(dst, src->element);
        src = src->next;
    }

    // return last inserted element
    return dst;
}

LList* insertAllBeforeLList(LList* linkedElement, LList* elements)
{
    LList* src = getTailLList(elements);
    LList* dst = linkedElement;

    while(src != NULL)
    {
        dst = insertBeforeLList(dst, src->element);
        src = src->prev;
    }

    // return last inserted element
    return dst;
}

LList* removeFromLList(LList* linkedElement)
{
    if (linkedElement == NULL) return NULL;

    // next element present ?
    if (linkedElement->next != NULL)
        // link next to prev
        linkedElement->next->prev = linkedElement->prev;

    // previous element present ?
    if (linkedElement->prev != NULL)
        // link prev to next
        linkedElement->prev->next = linkedElement->next;

    // return next element
    LList* result = linkedElement->next;

    // release element (don't do it so we can still access it)
    // free(linkedElement);

    return result;
}


void** llistToArray(LList* list)
{
    void** result;
    const int size = getSizeLList(list);

    if (size == 0)
    {
        result = malloc(sizeof(void*));
        *result = NULL;
    }
    else
    {
        int i;
        LList* l;

        result = malloc(sizeof(void*) * size);

        i = 0;
        l = list;
        while(l != NULL)
        {
            result[i++] = l->element;
            l = l ->next;
        }
    }

    return result;
}


bool arrayEquals(unsigned char* array1, unsigned char* array2, int size)
{
    int i;

    for (i = 0; i < size; i++)
        if (array1[i] != array2[i])
            return false;

    return true;
}

unsigned int swapNibble32(unsigned int value)
{
    return swapNibble16(value >> 16) | (swapNibble16(value) << 16);
}

unsigned short swapNibble16(unsigned short value)
{
    return swapNibble8(value >> 8) | (swapNibble8(value) << 8);
}

unsigned char swapNibble8(unsigned char value)
{
    return (value >> 4) | (value << 4);
}

unsigned short getShort(unsigned char* data, int offset)
{
    unsigned short res;

    res = data[offset + 0] << 0;
    res += data[offset + 1] << 8;

    return res;
}

unsigned int getInt16(unsigned char* data, int offset)
{
    unsigned int res;

    res = data[offset + 0] << 0;
    res += data[offset + 1]  << 8;

    return (int) res;
}

unsigned int getInt24(unsigned char* data, int offset)
{
    unsigned int res;

    res = data[offset + 0] << 0;
    res += data[offset + 1] << 8;
    res += data[offset + 2] << 16;

    return res;
}

unsigned int getInt(unsigned char* data, int offset)
{
    unsigned int res;

    res = data[offset + 0] << 0;
    res += data[offset + 1] << 8;
    res += data[offset + 2] << 16;
    res += data[offset + 3] << 24;

    return res;
}

void setInt(unsigned char* array, int offset, unsigned int value)
{
    array[offset + 0] = value >> 0;
    array[offset + 1] = value >> 8;
    array[offset + 2] = value >> 16;
    array[offset + 3] = value >> 24;
}

void setInt24(unsigned char* array, int offset, unsigned int value)
{
    array[offset + 0] = value >> 0;
    array[offset + 1] = value >> 8;
    array[offset + 2] = value >> 16;
}

void setInt16(unsigned char* array, int offset, unsigned int value)
{
    array[offset + 0] = value >> 0;
    array[offset + 1] = value >> 8;
}

char* getFileExtension(char* path)
{
    char* fext = strrchr(path, '.');

    if (fext) return fext + 1;

    // equivalent to ""
    return path + strlen(path);
}

unsigned int getFileSizeEx(FILE* f)
{
    unsigned int len;

    fseek(f, 0, SEEK_END);
    len = ftell(f);

    return len;
}

unsigned int getFileSize(char* file)
{
    unsigned int len;
    FILE * f;

    f = fopen(file, "rb");
    len = getFileSizeEx(f);
    fclose(f);

    return len;
}

unsigned char* readBinaryFile(char* fileName, int* size)
{
    FILE *f;
    unsigned char *data;

    f = fopen(fileName, "rb");

    if (!f)
    {
        printf("Error: couldn't open input file %s\n", fileName);
        // error
        return NULL;
    }

    *size = getFileSize(fileName);

    if (*size == 0)
    {
        printf("Error: empty file %s\n", fileName);
        // error
        return NULL;
    }

    data = malloc(*size);
    fread(data, 1, *size, f);
    fclose(f);

    return data;
}

bool writeBinaryFile(unsigned char* data, int size, char* fileName)
{
    return out(data, 0, size, 1, false, fileName);
}

unsigned char* inEx(FILE* fin, int inOffset, int size, int *outSize)
{
    unsigned char* result;

    // calculate size of output buffer
    *outSize = getFileSizeEx(fin) - inOffset;

    // nothing to read
    if (*outSize <= 0) return NULL;

    // alloc out buffer
    result = malloc(size);
    // and read
    *outSize = inEx2(fin, inOffset, size, result, 0);

    return result;
}

int inEx2(FILE* fin, int inOffset, int size, unsigned char* dest, int outOffset)
{
    unsigned char* d;
    int remain, l;

    fseek(fin, inOffset, SEEK_SET);
    d = dest + outOffset;

    remain = size;

    while (remain > 0)
    {
        l = fread(d, 1, size, fin);
        d += l;
        remain -= l;
    }

    return size - remain;
}

bool out(unsigned char* data, int inOffset, int size, int intSize, bool swap, char* out)
{
    int result;
    FILE *fout;

    fout = fopen(out, "wb");

    if (!fout)
    {
        printf("Error: couldn't create output file %s\n", out);

        // error
        return false;
    }

    result = outEx(data, inOffset, size, intSize, swap, fout, 0);
    fclose(fout);

    return result;
}

bool outEx(unsigned char* data, int inOffset, int size, int intSize, bool swap, FILE* fout, int outOffset)
{
    unsigned char* s;
    int remain, l;
    unsigned int v;

    fseek(fout, outOffset, SEEK_SET);
    s = data + inOffset;

    remain = size;

    while (remain > 0)
    {
        switch(intSize)
        {
            default:
                v = s[0];
                break;

            case 2:
                if (swap) v = (s[0] << 8) | (s[1] << 0);
                else v = (s[0] << 0) | (s[1] << 8);
                break;

            case 4:
                if (swap) v = (s[0] << 24) | (s[1] << 16) | (s[2] << 8) | (s[3] << 0);
                else v = (s[0] << 0) | (s[1] << 8) | (s[2] << 16) | (s[3] << 24);
                break;
        }

        l = fwrite(&v, 1, intSize, fout);
        s += l;
        remain -= l;
    }

    if (remain != 0) return false;
    else return true;
}


unsigned char* resample(unsigned char* data, int offset, int len, int inputRate, int outputRate, int align, int* outSize)
{
	char fname[L_tmpnam];
	tmpnam(fname);
    FILE* f = fopen(fname, "wb+");

    if (f == NULL)
    {
        printf("Error: cannot open file tmp.bin\n");
        return NULL;
    }

    const double step = (double) inputRate / (double) outputRate;

    double value;
    double lastSample;
    double sample = 0;
    unsigned char byte;
    double dOff;
    int off;
    int outOff;

    value = 0;
    lastSample = 0;
    off = 0;
    outOff = 0;
    for (dOff = 0; dOff < len; dOff += step)
    {
        sample = 0;

        // extrapolation
        if (step >= 1)
        {
            if (value < 0)
                sample += lastSample * -value;

            value += step;

            while (value > 0)
            {
                lastSample = (data[off + offset] & 0xFF) - 0x80;
                off++;

                if (value >= 1)
                    sample += lastSample;
                else
                    sample += lastSample * value;

                value--;
            }

            sample /= step;
        }
        else
        {
            sample = (data[(int) dOff + offset] & 0xFF) - 0x80;
        }

        byte = round(sample);
        fwrite(&byte, 1, 1, f);
        outOff++;
    }

    // do alignment
    if (align > 1)
    {
        const int mask = align - 1;
        const int size = align - (outOff & mask);

        if (size != align)
        {
            double reduce = sample / size;
            int i;

            for (i = 0; i < size; i++)
            {
                sample -= reduce;
                byte = round(sample);
                fwrite(&byte, 1, 1, f);
                outOff++;
            }
        }
    }

    unsigned char* result = inEx(f, 0, outOff, outSize);

    fclose(f);

    return result;
}
