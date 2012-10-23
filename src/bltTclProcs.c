#define BUILD_BLT_TCL_PROCS 1
#include <bltInt.h>

/* !BEGIN!: Do not edit below this line. */
extern BltTclIntProcs bltTclIntProcs;

static BltTclStubHooks bltTclStubHooks = {
    &bltTclIntProcs
};

BltTclProcs bltTclProcs = {
    TCL_STUB_MAGIC,
    &bltTclStubHooks,
    NULL, /* 0 */
    Blt_AllocInit, /* 1 */
    Blt_Chain_Init, /* 2 */
    Blt_Chain_Create, /* 3 */
    Blt_Chain_Destroy, /* 4 */
    Blt_Chain_NewLink, /* 5 */
    Blt_Chain_AllocLink, /* 6 */
    Blt_Chain_Append, /* 7 */
    Blt_Chain_Prepend, /* 8 */
    Blt_Chain_Reset, /* 9 */
    Blt_Chain_InitLink, /* 10 */
    Blt_Chain_LinkAfter, /* 11 */
    Blt_Chain_LinkBefore, /* 12 */
    Blt_Chain_UnlinkLink, /* 13 */
    Blt_Chain_DeleteLink, /* 14 */
    Blt_Chain_GetNthLink, /* 15 */
    Blt_Chain_Sort, /* 16 */
    Blt_Chain_IsBefore, /* 17 */
    Blt_List_Init, /* 18 */
    Blt_List_Reset, /* 19 */
    Blt_List_Create, /* 20 */
    Blt_List_Destroy, /* 21 */
    Blt_List_CreateNode, /* 22 */
    Blt_List_DeleteNode, /* 23 */
    Blt_List_Append, /* 24 */
    Blt_List_Prepend, /* 25 */
    Blt_List_LinkAfter, /* 26 */
    Blt_List_LinkBefore, /* 27 */
    Blt_List_UnlinkNode, /* 28 */
    Blt_List_GetNode, /* 29 */
    Blt_List_DeleteNodeByKey, /* 30 */
    Blt_List_GetNthNode, /* 31 */
    Blt_List_Sort, /* 32 */
    blt_table_release_tags, /* 33 */
    blt_table_exists, /* 34 */
    blt_table_create, /* 35 */
    blt_table_open, /* 36 */
    blt_table_close, /* 37 */
    blt_table_same_object, /* 38 */
    blt_table_row_get_label_table, /* 39 */
    blt_table_column_get_label_table, /* 40 */
    blt_table_row_find_by_label, /* 41 */
    blt_table_column_find_by_label, /* 42 */
    blt_table_row_find_by_index, /* 43 */
    blt_table_column_find_by_index, /* 44 */
    blt_table_row_set_label, /* 45 */
    blt_table_column_set_label, /* 46 */
    blt_table_column_convert_name_to_type, /* 47 */
    blt_table_column_set_type, /* 48 */
    blt_table_name_of_type, /* 49 */
    blt_table_column_set_tag, /* 50 */
    blt_table_row_set_tag, /* 51 */
    blt_table_row_create, /* 52 */
    blt_table_column_create, /* 53 */
    blt_table_row_extend, /* 54 */
    blt_table_column_extend, /* 55 */
    blt_table_row_delete, /* 56 */
    blt_table_column_delete, /* 57 */
    blt_table_row_move, /* 58 */
    blt_table_column_move, /* 59 */
    blt_table_get_obj, /* 60 */
    blt_table_set_obj, /* 61 */
    blt_table_get_string, /* 62 */
    blt_table_set_string, /* 63 */
    blt_table_append_string, /* 64 */
    blt_table_get_double, /* 65 */
    blt_table_set_double, /* 66 */
    blt_table_get_long, /* 67 */
    blt_table_set_long, /* 68 */
    blt_table_get_value, /* 69 */
    blt_table_set_value, /* 70 */
    blt_table_unset_value, /* 71 */
    blt_table_value_exists, /* 72 */
    blt_table_row_find_tag_table, /* 73 */
    blt_table_column_find_tag_table, /* 74 */
    blt_table_row_tags, /* 75 */
    blt_table_column_tags, /* 76 */
    blt_table_tags_are_shared, /* 77 */
    blt_table_row_has_tag, /* 78 */
    blt_table_column_has_tag, /* 79 */
    blt_table_row_forget_tag, /* 80 */
    blt_table_column_forget_tag, /* 81 */
    blt_table_row_unset_tag, /* 82 */
    blt_table_column_unset_tag, /* 83 */
    blt_table_row_first_tag, /* 84 */
    blt_table_column_first_tag, /* 85 */
    blt_table_column_first, /* 86 */
    blt_table_column_next, /* 87 */
    blt_table_row_first, /* 88 */
    blt_table_row_next, /* 89 */
    blt_table_row_spec, /* 90 */
    blt_table_column_spec, /* 91 */
    blt_table_row_iterate, /* 92 */
    blt_table_column_iterate, /* 93 */
    blt_table_row_iterate_objv, /* 94 */
    blt_table_column_iterate_objv, /* 95 */
    blt_table_free_iterator_objv, /* 96 */
    blt_table_row_iterate_all, /* 97 */
    blt_table_column_iterate_all, /* 98 */
    blt_table_row_first_tagged, /* 99 */
    blt_table_column_first_tagged, /* 100 */
    blt_table_row_next_tagged, /* 101 */
    blt_table_column_next_tagged, /* 102 */
    blt_table_row_find, /* 103 */
    blt_table_column_find, /* 104 */
    blt_table_list_rows, /* 105 */
    blt_table_list_columns, /* 106 */
    blt_table_row_clear_tags, /* 107 */
    blt_table_column_clear_tags, /* 108 */
    blt_table_row_clear_traces, /* 109 */
    blt_table_column_clear_traces, /* 110 */
    blt_table_create_trace, /* 111 */
    blt_table_column_create_trace, /* 112 */
    blt_table_column_create_tag_trace, /* 113 */
    blt_table_row_create_trace, /* 114 */
    blt_table_row_create_tag_trace, /* 115 */
    blt_table_delete_trace, /* 116 */
    blt_table_row_create_notifier, /* 117 */
    blt_table_row_create_tag_notifier, /* 118 */
    blt_table_column_create_notifier, /* 119 */
    blt_table_column_create_tag_notifier, /* 120 */
    blt_table_delete_notifier, /* 121 */
    blt_table_sort_init, /* 122 */
    blt_table_sort_rows, /* 123 */
    blt_table_sort_rows_subset, /* 124 */
    blt_table_sort_finish, /* 125 */
    blt_table_get_compare_proc, /* 126 */
    blt_table_row_get_map, /* 127 */
    blt_table_column_get_map, /* 128 */
    blt_table_row_set_map, /* 129 */
    blt_table_column_set_map, /* 130 */
    blt_table_restore, /* 131 */
    blt_table_file_restore, /* 132 */
    blt_table_register_format, /* 133 */
    blt_table_unset_keys, /* 134 */
    blt_table_get_keys, /* 135 */
    blt_table_set_keys, /* 136 */
    blt_table_key_lookup, /* 137 */
    blt_table_column_get_limits, /* 138 */
    Blt_Pool_Create, /* 139 */
    Blt_Pool_Destroy, /* 140 */
    Blt_Tree_GetKey, /* 141 */
    Blt_Tree_GetKeyFromNode, /* 142 */
    Blt_Tree_GetKeyFromInterp, /* 143 */
    Blt_Tree_CreateNode, /* 144 */
    Blt_Tree_CreateNodeWithId, /* 145 */
    Blt_Tree_DeleteNode, /* 146 */
    Blt_Tree_MoveNode, /* 147 */
    Blt_Tree_GetNode, /* 148 */
    Blt_Tree_FindChild, /* 149 */
    Blt_Tree_NextNode, /* 150 */
    Blt_Tree_PrevNode, /* 151 */
    Blt_Tree_FirstChild, /* 152 */
    Blt_Tree_LastChild, /* 153 */
    Blt_Tree_IsBefore, /* 154 */
    Blt_Tree_IsAncestor, /* 155 */
    Blt_Tree_PrivateValue, /* 156 */
    Blt_Tree_PublicValue, /* 157 */
    Blt_Tree_GetValue, /* 158 */
    Blt_Tree_ValueExists, /* 159 */
    Blt_Tree_SetValue, /* 160 */
    Blt_Tree_UnsetValue, /* 161 */
    Blt_Tree_AppendValue, /* 162 */
    Blt_Tree_ListAppendValue, /* 163 */
    Blt_Tree_GetArrayValue, /* 164 */
    Blt_Tree_SetArrayValue, /* 165 */
    Blt_Tree_UnsetArrayValue, /* 166 */
    Blt_Tree_AppendArrayValue, /* 167 */
    Blt_Tree_ListAppendArrayValue, /* 168 */
    Blt_Tree_ArrayValueExists, /* 169 */
    Blt_Tree_ArrayNames, /* 170 */
    Blt_Tree_GetValueByKey, /* 171 */
    Blt_Tree_SetValueByKey, /* 172 */
    Blt_Tree_UnsetValueByKey, /* 173 */
    Blt_Tree_AppendValueByKey, /* 174 */
    Blt_Tree_ListAppendValueByKey, /* 175 */
    Blt_Tree_ValueExistsByKey, /* 176 */
    Blt_Tree_FirstKey, /* 177 */
    Blt_Tree_NextKey, /* 178 */
    Blt_Tree_Apply, /* 179 */
    Blt_Tree_ApplyDFS, /* 180 */
    Blt_Tree_ApplyBFS, /* 181 */
    Blt_Tree_SortNode, /* 182 */
    Blt_Tree_Exists, /* 183 */
    Blt_Tree_Open, /* 184 */
    Blt_Tree_Close, /* 185 */
    Blt_Tree_Attach, /* 186 */
    Blt_Tree_GetFromObj, /* 187 */
    Blt_Tree_Size, /* 188 */
    Blt_Tree_CreateTrace, /* 189 */
    Blt_Tree_DeleteTrace, /* 190 */
    Blt_Tree_CreateEventHandler, /* 191 */
    Blt_Tree_DeleteEventHandler, /* 192 */
    Blt_Tree_RelabelNode, /* 193 */
    Blt_Tree_RelabelNodeWithoutNotify, /* 194 */
    Blt_Tree_NodeIdAscii, /* 195 */
    Blt_Tree_NodePath, /* 196 */
    Blt_Tree_NodeRelativePath, /* 197 */
    Blt_Tree_NodePosition, /* 198 */
    Blt_Tree_ClearTags, /* 199 */
    Blt_Tree_HasTag, /* 200 */
    Blt_Tree_AddTag, /* 201 */
    Blt_Tree_RemoveTag, /* 202 */
    Blt_Tree_ForgetTag, /* 203 */
    Blt_Tree_TagHashTable, /* 204 */
    Blt_Tree_TagTableIsShared, /* 205 */
    Blt_Tree_NewTagTable, /* 206 */
    Blt_Tree_FirstTag, /* 207 */
    Blt_Tree_DumpNode, /* 208 */
    Blt_Tree_Dump, /* 209 */
    Blt_Tree_DumpToFile, /* 210 */
    Blt_Tree_Restore, /* 211 */
    Blt_Tree_RestoreFromFile, /* 212 */
    Blt_Tree_Depth, /* 213 */
    Blt_Tree_RegisterFormat, /* 214 */
    Blt_Tree_RememberTag, /* 215 */
    Blt_InitHashTable, /* 216 */
    Blt_InitHashTableWithPool, /* 217 */
    Blt_DeleteHashTable, /* 218 */
    Blt_DeleteHashEntry, /* 219 */
    Blt_FirstHashEntry, /* 220 */
    Blt_NextHashEntry, /* 221 */
    Blt_HashStats, /* 222 */
    Blt_VecMin, /* 223 */
    Blt_VecMax, /* 224 */
    Blt_AllocVectorId, /* 225 */
    Blt_SetVectorChangedProc, /* 226 */
    Blt_FreeVectorId, /* 227 */
    Blt_GetVectorById, /* 228 */
    Blt_NameOfVectorId, /* 229 */
    Blt_NameOfVector, /* 230 */
    Blt_VectorNotifyPending, /* 231 */
    Blt_CreateVector, /* 232 */
    Blt_CreateVector2, /* 233 */
    Blt_GetVector, /* 234 */
    Blt_GetVectorFromObj, /* 235 */
    Blt_VectorExists, /* 236 */
    Blt_ResetVector, /* 237 */
    Blt_ResizeVector, /* 238 */
    Blt_DeleteVectorByName, /* 239 */
    Blt_DeleteVector, /* 240 */
    Blt_ExprVector, /* 241 */
    Blt_InstallIndexProc, /* 242 */
    Blt_VectorExists2, /* 243 */
};

/* !END!: Do not edit above this line. */
