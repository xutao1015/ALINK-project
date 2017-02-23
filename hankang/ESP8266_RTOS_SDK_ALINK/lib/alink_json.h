
/* vim: set et ts=3 sw=3 sts=3 ft=c:
 *
 * Copyright (C) 2012, 2013, 2014 James McLaughlin et al.  All rights reserved.
 * https://github.com/udp/json-parser
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _JSON_H
#define _JSON_H

#ifndef json_char
#define json_char char
#endif

#ifndef json_int_t
#ifndef _MSC_VER
#ifndef _PLATFORM_THREADX_
#include <inttypes.h>
#endif
#define json_int_t long long 
#else
#define json_int_t __int64
#endif
#endif

#ifndef _PLATFORM_THREADX_
#include <stdlib.h>
#endif

#ifdef __cplusplus

#include <string.h>

extern "C" {

#endif

    typedef struct {
        unsigned long max_memory;
        int settings;

        /* Custom allocator support (leave null to use malloc/free)
         */

        void *(*json_mem_alloc) (size_t, int zero, void *user_data);
        void (*json_mem_free) (void *, void *user_data);

        void *user_data;        /* will be passed to mem_alloc and mem_free */

        size_t value_extra;     /* how much extra space to allocate for values? */

    } json_settings;
    typedef enum {
        json_none,
        json_object_e,
        json_array,
        json_integer,
        json_double,
        json_string,
        json_boolean,
        json_null
    } json_type_e;

    extern const struct _json_value json_value_none;

    typedef struct _json_object_entry {
        json_char *name;
        unsigned int name_length;

        struct _json_value *value;

    } json_object_entry;

    typedef struct _json_value {
        struct _json_value *parent;

        json_type_e type;
        unsigned int begin, end;

        union {
            int boolean;
            json_int_t integer;
            double dbl;

            struct {
                unsigned int length;
                json_char *ptr; /* null terminated */

            } string;

            struct {
                unsigned int length;

                json_object_entry *values;

#if defined(__cplusplus) && __cplusplus >= 201103L
                 decltype(values) begin() const {
                    return values;
                } decltype(values) end() const {
                    return values + length;
                }
#endif
            }
            object;

            struct {
                unsigned int length;
                struct _json_value **values;

#if defined(__cplusplus) && __cplusplus >= 201103L
                 decltype(values) begin() const {
                    return values;
                } decltype(values) end() const {
                    return values + length;
                }
#endif
            }
            array;

        }
        u;

        union {
            struct _json_value *next_alloc;
            void *object_mem;

        } _reserved;

#ifdef JSON_TRACK_SOURCE

        /* Location of the value in the source JSON
         */
        unsigned int line, col;

#endif

        /* Some C++ operator sugar */

#ifdef __cplusplus

      public:

         inline _json_value() {
            memset(this, 0, sizeof(_json_value));
        } inline const struct _json_value &operator [] (int index) const {
            if (type != json_array || index < 0
                || ((unsigned int) index) >= u.array.length) {
                return json_value_none;
            }

            return *u.array.values[index];
        } inline const struct _json_value &operator [] (const char *index) const {
            if (type != json_object_e)
                return json_value_none;

            for (unsigned int i = 0; i < u.object.length; ++i)
                if (!strcmp(u.object.values[i].name, index))
                    return *u.object.values[i].value;

            return json_value_none;
        } inline operator            const char *() const {
            switch (type)
            {
            case json_string:
                return u.string.ptr;

                default:return "";
            };
        }

        inline operator            json_int_t() const {
            switch (type)
            {
            case json_integer:
                return u.integer;

                case json_double:return (json_int_t) u.dbl;

                default:return 0;
            };
        }

        inline operator            bool() const {
            if (type != json_boolean)
                return false;

            return u.boolean != 0;
        } inline operator            double () const {
            switch (type)
            {
            case json_integer:
                return (double) u.integer;

                case json_double:return u.dbl;

                default:return 0;
            };
        }

#endif

    }
    json_value;

    json_value *json_parse(const json_char * json, size_t length);

#define json_error_max 128
    json_value *json_parse_ex(json_settings * settings,
                              const json_char * json,
                              size_t length, char *error);

    void json_value_free(json_value *);

/* Not usually necessary, unless you used a custom mem_alloc and now want to
 * use a custom mem_free.
 */
    void json_value_free_ex(json_settings * settings, json_value *);

    json_value *json_object_object_get_e(json_value * p, char *name);
    char *json_object_to_json_string_e(json_value * p);
    unsigned int json_object_to_json_string_length(json_value * p);
    int json_object_get_int_e(json_value * p);

    int json_array_get_length(json_value * p);
    json_value *json_array_get_object(json_value * p, size_t pos);
    char *alloc_str_from_json(const char *json_str, json_value * p);

#ifdef __cplusplus
}                               /* extern "C" */
#endif

#endif
