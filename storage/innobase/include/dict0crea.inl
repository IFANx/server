/*****************************************************************************

Copyright (c) 1996, 2016, Oracle and/or its affiliates. All Rights Reserved.
Copyright (c) 2019, MariaDB Corporation.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA

*****************************************************************************/

/**************************************************//**
@file include/dict0crea.ic
Database object creation

Created 1/8/1996 Heikki Tuuri
*******************************************************/

#include "ha_prototypes.h"

#include "mem0mem.h"

/********************************************************************//**
Generate a foreign key constraint name when it was not named by the user.
A generated constraint has a name of the format dbname/tablename_ibfk_NUMBER,
where the numbers start from 1, and are given locally for this table, that is,
the number is not global, as it used to be before MySQL 4.0.18.  */
UNIV_INLINE
dberr_t
dict_create_add_foreign_id(
/*=======================*/
	ulint*		id_nr,	/*!< in/out: number to use in id generation;
				incremented if used */
	const char*	name,	/*!< in: table name */
	dict_foreign_t*	foreign)/*!< in/out: foreign key */
{
	DBUG_ENTER("dict_create_add_foreign_id");

	if (foreign->id == NULL) {
		/* Generate a new constraint id */
		ulint	namelen	= strlen(name);
		char*	id	= static_cast<char*>(
					mem_heap_alloc(foreign->heap,
						       namelen + 20));

		if (dict_table_t::is_temporary_name(name)) {

			/* no overflow if number < 1e13 */
//			sprintf(id, "%s_ibfk_%lu", name,
//				(ulong) (*id_nr)++);
		} else {
			char	table_name[MAX_TABLE_NAME_LEN + 21];
			uint	errors = 0;

			strncpy(table_name, name, (sizeof table_name) - 1);
			table_name[(sizeof table_name) - 1] = '\0';

			innobase_convert_to_system_charset(
				strchr(table_name, '/') + 1,
				strchr(name, '/') + 1,
				MAX_TABLE_NAME_LEN, &errors);

			if (errors) {
				strncpy(table_name, name,
					(sizeof table_name) - 1);
				table_name[(sizeof table_name) - 1] = '\0';
			}

			/* no overflow if number < 1e13 */
//			sprintf(id, "%s_ibfk_%lu", table_name,
//				(ulong) (*id_nr)++);

			if (innobase_check_identifier_length(
				strchr(id,'/') + 1)) {
				DBUG_RETURN(DB_IDENTIFIER_TOO_LONG);
			}
		}
		foreign->id = id;

		DBUG_PRINT("dict_create_add_foreign_id",
			   ("generated foreign id: %s", id));
	}


	DBUG_RETURN(DB_SUCCESS);
}

/** Compose a column number for a virtual column, stored in the "POS" field
of Sys_columns. The column number includes both its virtual column sequence
(the "nth" virtual column) and its actual column position in original table
@param[in]	v_pos		virtual column sequence
@param[in]	col_pos		column position in original table definition
@return composed column position number */
UNIV_INLINE
ulint
dict_create_v_col_pos(
	ulint	v_pos,
	ulint	col_pos)
{
	ut_ad(v_pos <= REC_MAX_N_FIELDS);
	ut_ad(col_pos <= REC_MAX_N_FIELDS);

	return(((v_pos + 1) << 16) + col_pos);
}

/** Get the column number for a virtual column (the column position in
original table), stored in the "POS" field of Sys_columns
@param[in]	pos		virtual column position
@return column position in original table */
UNIV_INLINE
ulint
dict_get_v_col_mysql_pos(
	ulint	pos)
{
	return(pos & 0xFFFF);
}

/** Get a virtual column sequence (the "nth" virtual column) for a
virtual column, stord in the "POS" field of Sys_columns
@param[in]	pos		virtual column position
@return virtual column sequence */
UNIV_INLINE
ulint
dict_get_v_col_pos(
	ulint	pos)
{
	return((pos >> 16) - 1);
}
