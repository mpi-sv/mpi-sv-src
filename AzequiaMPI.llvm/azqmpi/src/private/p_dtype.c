/* _________________________________________________________________________
 |                                                                       |
 |  Azequia (embedded) Message Passing Interface   ( AzequiaMPI )        |
 |                                                                       |
 |  Authors: DSP Systems Group                                           |
 |           http://gsd.unex.es                                          |
 |           University of Extremadura                                   |
 |           Caceres, Spain                                              |
 |           jarico@unex.es                                              |
 |                                                                       |
 |  Date:    Sept 22, 2008                                               |
 |                                                                       |
 |  Description:                                                         |
 |                                                                       |
 |                                                                       |
 |_______________________________________________________________________| */

/*----------------------------------------------------------------/
 /   Declaration of public functions implemented by this module    /
 /----------------------------------------------------------------*/
#include <p_dtype.h>

/*----------------------------------------------------------------/
 /   Declaration of public functions used by this module           /
 /----------------------------------------------------------------*/
#if defined (__OSI)
#include <osi.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#endif

#include <azq_types.h>

#include <env.h>
#include <p_config.h>

/*----------------------------------------------------------------/
 /                  Definition of private constants                /
 /----------------------------------------------------------------*/
/* Size of each type in MAUs */
#define MPI_CHAR_LEN                (sizeof(char))
#define MPI_SHORT_LEN               (sizeof(short))
#define MPI_INT_LEN                 (sizeof(int))
#define	MPI_LONG_LEN                (sizeof(long))
#define	MPI_UNS_CHAR_LEN            (sizeof(unsigned char))
#define	MPI_UNS_SHORT_LEN           (sizeof(unsigned short))
#define	MPI_UNSIGNED_LEN            (sizeof(unsigned int))
#define	MPI_UNS_LONG_LEN            (sizeof(unsigned long))
#define	MPI_FLOAT_LEN               (sizeof(float))
#define	MPI_DOUBLE_LEN              (sizeof(double))
#define	MPI_LONG_DOUBLE_LEN         (sizeof(long double))

#define MPI_LONG_LONG_LEN           (sizeof(long long))
#define MPI_LONG_LONG_INT_LEN       (sizeof(long long int))
#define MPI_UNSIGNED_LONG_LONG_LEN  (sizeof(unsigned long long))

#define MPI_2INT_LEN                (sizeof(struct{int         a; int    b;}))
#define MPI_SHORT_INT_LEN           (sizeof(struct{short       a; int    b;}))
#define MPI_LONG_INT_LEN            (sizeof(struct{long        a; int    b;}))
#define MPI_FLOAT_INT_LEN           (sizeof(struct{float       a; int    b;}))
#define MPI_DOUBLE_INT_LEN          (sizeof(struct{double      a; int    b;}))
#define MPI_LONG_DOUBLE_INT_LEN     (sizeof(struct{long double a; int    b;}))

/* Fortran datatypes */
#define MPI_COMPLEX_LEN             (sizeof(struct{float       r; float  i;}))
#define MPI_DOUBLE_COMPLEX_LEN      (sizeof(struct{double      r; double i;}))

#define	MPI_BYTE_LEN                1
#define MPI_PACKED_LEN              1

/* Datatype length for basic datatypes. It is the extent too */
static int datatype_len[] = { MPI_CHAR_LEN, MPI_SHORT_LEN,
MPI_INT_LEN,
MPI_LONG_LEN,
MPI_LONG_LONG_INT_LEN,
MPI_LONG_LONG_LEN,
MPI_UNS_CHAR_LEN,
MPI_UNS_SHORT_LEN,
MPI_UNSIGNED_LEN,
MPI_UNS_LONG_LEN,
MPI_UNSIGNED_LONG_LONG_LEN,
MPI_FLOAT_LEN,
MPI_DOUBLE_LEN,
MPI_LONG_DOUBLE_LEN,
MPI_2INT_LEN,
MPI_SHORT_INT_LEN,
MPI_LONG_INT_LEN,
MPI_FLOAT_INT_LEN,
MPI_DOUBLE_INT_LEN,
MPI_LONG_DOUBLE_INT_LEN,
MPI_COMPLEX_LEN,
MPI_DOUBLE_COMPLEX_LEN,
MPI_BYTE_LEN,
MPI_PACKED_LEN
};

#define DATATYPE_LEN(datatype) (datatype_len[datatype])

/*----------------------------------------------------------------/
 /                  Definition of opaque types                     /
 /----------------------------------------------------------------*/
/* Table of datatypes */
struct DTypeTable {
	Mpi_P_Datatype *Dtypes;
};

/* Table of basic datatypes */
Mpi_P_Datatype BasicDTypeTable[BASIC_DATATYPES];

/*-------------------------------------------------------/
 /         Declaration of private interface               /
 /-------------------------------------------------------*/PRIVATE int rec_pack(
		void *inbuf, void *outbuf, Mpi_P_Datatype *datatype, int *position,
		Mpi_P_Aint *displ);
PRIVATE int rec_unpack(void *inbuf, void *outbuf, Mpi_P_Datatype *datatype,
		int *position, Mpi_P_Aint *displ);
PRIVATE int rec_packSize(Mpi_P_Datatype *datatype, int *size);

#ifdef DEBUG_MODE
PRIVATE int printDatatypeTable (DTypeTable *dtypetable);
PRIVATE int printDatatype (Mpi_P_Datatype *dtype);
#endif
#include <stdlib.h>

/*-------------------------------------------------------/
 /                 Private interface                      /
 /-------------------------------------------------------*/
/*
 * rec_pack. Recursive routine for packing datatyped data
 */PRIVATE int rec_pack(void *inbuf, void *outbuf, Mpi_P_Datatype *datatype,
		int *position, Mpi_P_Aint *displ) {

	int i, j;
	Mpi_P_Datatype *subtype;
	Mpi_P_Aint realdisp;

	if (datatype->State & DTYPE_MEM_CONTIG) {

		memcpy(outbuf + (*position), inbuf + (Mpi_P_Aint) (*displ),
				datatype->Extent);
		(*position) += datatype->Extent;

	} else {

		for (i = 0; i < datatype->Count; i++) {

			subtype = datatype->Types[i];
			if (((int) subtype != MPI_P_LB) && ((int) subtype != MPI_P_UB)) {

				for (j = 0; j < datatype->BlkLen[i]; j++) {
					realdisp = ((Mpi_P_Aint) (datatype->Displs[i]
							+ (j * subtype->Extent)) + (Mpi_P_Aint) (*displ));
					rec_pack(inbuf, outbuf, subtype, position, &realdisp);
				}

			}

		}

	}

	return DTP_E_OK;
}

/*
 * rec_unpack. Recursive routine for unpacking datatyped data
 */PRIVATE int rec_unpack(void *inbuf, void *outbuf, Mpi_P_Datatype *datatype,
		int *position, Mpi_P_Aint *displ) {

	int i, j;
	Mpi_P_Datatype *subtype;
	Mpi_P_Aint realdisp;

	if (datatype->State & DTYPE_MEM_CONTIG) {

		memcpy(outbuf + (Mpi_P_Aint) (*displ), inbuf + (*position),
				datatype->Extent);
		(*position) += datatype->Extent;

	} else {

		for (i = 0; i < datatype->Count; i++) {

			subtype = datatype->Types[i];
			if (((int) subtype != MPI_P_LB) && ((int) subtype != MPI_P_UB)) {

				for (j = 0; j < datatype->BlkLen[i]; j++) {
					realdisp = (datatype->Displs[i] + (j * subtype->Extent))
							+ *displ;
					rec_unpack(inbuf, outbuf, subtype, position, &realdisp);
				}

			}

		}

	}

	return DTP_E_OK;
}

/*
 * rec_packsize. Recursive routine for calculate the size of packed datatyped data
 */PRIVATE int rec_packSize(Mpi_P_Datatype *datatype, int *size) {

	int i;
	Mpi_P_Datatype *subtype;
	int realsize;

	if (datatype->State & DTYPE_MEM_CONTIG) {

		*size = datatype->Extent;
		//*size = datatype->Size;

	} else {

		for (i = 0; i < datatype->Count; i++) {

			subtype = datatype->Types[i];
			if (((int) subtype != MPI_P_LB) && ((int) subtype != MPI_P_UB)) {
				realsize = 0;
				rec_packSize(subtype, &realsize);
				*size += realsize * datatype->BlkLen[i];
			}

		}

	}

	return DTP_E_OK;
}

/*-------------------------------------------------------/
 /                 Public interface                       /
 /-------------------------------------------------------*/
/*
 *  dtypeAllocTable():
 *   Allocate a datatype table with a number of datatype structs
 */
int dtypeAllocTable(DTypeTable_t *dtypetable) {

	DTypeTable *typetab;
	Mpi_P_Datatype *dtype;
	int i, error;

//fprintf(stdout, "\ndtypeAllocTable(%p): BEGIN\n", self()); fflush(stdout);

	if (posix_memalign(&typetab, CACHE_LINE_SIZE, sizeof(struct DTypeTable)))
		return DTP_E_EXHAUST;

	objInit(&dtype, PREALLOC_MAX_DTYPE_BLOCKS, sizeof(Mpi_P_Datatype),
			PREALLOC_DTYPE_COUNT, BLOCK_DTYPE_COUNT, &error);
	if (error)
		goto exception;

	typetab->Dtypes = dtype;

	*dtypetable = typetab;

#ifdef DEBUG_MODE
	printDatatypeTable(*dtypetable);
#endif

//fprintf(stdout, "dtypeAllocTable(%p): END\n", self()); fflush(stdout);

	return (DTP_E_OK);

	exception: free(typetab);
	return -1;
}

/*
 *  dtypeFreeTable():
 *   Free the datatypes table
 */
int dtypeFreeTable(DTypeTable_t *dtypetable) {

	objFinalize(&(*dtypetable)->Dtypes);

	free(*dtypetable);

	*dtypetable = (DTypeTable *) NULL;

	return DTP_E_OK;
}

/*
 *  dtypeCreateDefault():
 *   Fill the global basic datatype table
 */
int dtypeCreateDefault(DTypeTable *dtypetable) {

	Mpi_P_Datatype *dtype;
	int i;

	memset((void *) BasicDTypeTable, 0,
			sizeof(Mpi_P_Datatype) * BASIC_DATATYPES);

	for (i = 0; i < BASIC_DATATYPES; i++) {

		dtype = &BasicDTypeTable[i];

		dtype->State = DTYPE_BASIC | DTYPE_COMMITED | DTYPE_ALLOCATED
				| DTYPE_MEM_CONTIG;
		dtype->BasicSign = i;
		dtype->BasicDispl = 0;
		dtype->Elements = 1;
		dtype->Blocks = 1;
		dtype->Count = 1;
		dtype->Size = DATATYPE_LEN(i);
		dtype->Lb = 0;
		dtype->Ub = DATATYPE_LEN(i);
		dtype->Extent = dtype->Ub - dtype->Lb;
		dtype->PackSize = dtype->Extent;
		dtype->Refs = 1;

		dtype->Index = i;
	}

#ifdef DEBUG_MODE
	printDatatypeTable(dtypetable);
#endif

	return DTP_E_OK;
}

/*
 *  dtypeDeleteDefault():
 */
int dtypeDeleteDefault(DTypeTable *dtypetable) {
	return DTP_E_OK;
}

/*-------------------------------------------------------------------------/
 /    Implementation of exported interface for datatypes management         /
 /-------------------------------------------------------------------------*/
/*
 *  dtypeCreate
 *   Create a datatype
 */
//**************************MPISE********************************
//Line 451 	memcpy(newt->Displs, displs, displs_sz) always reports
// memory bound out, there can be some doubt that klee supports
// posix_memalign.
//***************************************************************************
int dtypeCreateStruct(DTypeTable *dtypetable, int count, int *blklen,
		Mpi_P_Aint *displs, Mpi_P_Datatype **dtypes, int kind,
		Mpi_P_Datatype_t *datatypenew) {

	int i, j;
	Mpi_P_Datatype *newt;
	int signs_sz, displs_sz, items_sz;
	Mpi_P_Aint tmp_lb, tmp_ub;
	Mpi_P_Aint new_lb, new_ub;
	int found_lb, found_ub;
	int epsilon;
	Mpi_P_Aint min_disp, max_disp, max_calc;
	int old_extent;
	int idx;

	/* 1. Allocate object */
	/*
	 if (0 > objAlloc(dtypetable->Dtypes, &newt, &idx)) {
	 *datatypenew = (Mpi_P_Datatype *)NULL;
	 return(DTP_E_EXHAUST);
	 }
	 */
	objAlloc((Object_t )dtypetable->Dtypes, &newt, &idx);
	memset(newt, 0, sizeof(struct Mpi_P_Datatype));
	newt->State = DTYPE_ALLOCATED;

	newt->Index = idx;

	/* 3. Allocate space for new datatype (blocklengths, displacements and types) */
	signs_sz = count * sizeof(Mpi_P_Datatype *);
	displs_sz = count * sizeof(Mpi_P_Aint);
	items_sz = count * sizeof(int);

	/* 2. Allocate space for types, displacements and block lengths */
	if (posix_memalign((void *) &newt->Types, CACHE_LINE_SIZE, signs_sz)) {
	//newt->Types = memalign(CACHE_LINE_SIZE, signs_sz);
	//if (!newt->Types) {
		newt->State = 0;
		return DTP_E_EXHAUST;
	}

	if (posix_memalign((void *) &newt->Displs, CACHE_LINE_SIZE, displs_sz)) {
	//newt->Displs = memalign(CACHE_LINE_SIZE, displs_sz);
	//if (!newt->Displs) {
		free(newt->Types);
		newt->State = 0;
		return DTP_E_EXHAUST;
	}

  if (posix_memalign((void *) &newt->BlkLen, CACHE_LINE_SIZE, items_sz)) {
	//newt->BlkLen = memalign(CACHE_LINE_SIZE, items_sz);
	//if (!newt->BlkLen) {
		free(newt->Types);
		free(newt->Displs);
		newt->State = 0;
		return DTP_E_EXHAUST;
	}

	newt->Count = count;

	/* 4.1. Copy the datatype info. Some kinds of datatype creation need replication */
	if ((kind == DTYPE_CONTIGUOUS) || (kind == DTYPE_STRUCT)) {
         memcpy((void*)newt->Types, (void*)dtypes, signs_sz);
		//memcpy((Mpi_P_Datatype **) newt->Types, dtypes, signs_sz);
	} else {
		for (i = 0; i < count; i++)
			newt->Types[i] = *dtypes;
	}

	/* 4.2. Displacements needed in bytes, so some kinds of datatypes need convertion */
	for (i = 0; i < count; i++) {
		if (((int) (newt->Types[i]) != MPI_P_LB)
				&& ((int) (newt->Types[i]) != MPI_P_UB)) {
			break;
		}
	}
	old_extent = newt->Types[i]->Extent;

	if (kind == DTYPE_INDEXED) {
		for (i = 0; i < count; i++)
//newt->Displs[i] = (Mpi_P_Aint)(displs[i]) * old_extent;
			newt->Displs[i] = displs[i] * old_extent;
	} else if (kind == DTYPE_HINDEXED) {
		for (i = 0; i < count; i++)
//newt->Displs[i] = (Mpi_P_Aint)(displs[i]);
			newt->Displs[i] = displs[i];
	} else if (kind == DTYPE_VECTOR) {
		for (i = 0; i < count; i++)
//newt->Displs[i] = ((Mpi_P_Aint)(*displs) * old_extent) * i;
			newt->Displs[i] = ((*displs) * old_extent) * i;
	} else if (kind == DTYPE_HVECTOR) {
		for (i = 0; i < count; i++)
//newt->Displs[i] = (Mpi_P_Aint)(*displs) * i;
			newt->Displs[i] = (*displs) * i;
	} else {
		if (displs) {
			//for (i = 0; i < count; i++){
			//newt->Displs[i] = (Mpi_P_Aint) (*displs) * i;
			//newt->Displs[i] = displs[i];
			//klee_debug("displs/newt->Displs:%lld\n", displs[0]);
			//klee_debug("displs/newt->Displs:%ld\n", displs[1]);
			//klee_debug("displs/newt->Displs:%ld\n", displs[1]);
			//printf("%x\n%ld\n", displs, *displs);
			//}
			//memcpy((Mpi_P_Aint *)(newt->Displs), (Mpi_P_Aint *)displs, displs_sz);
			memcpy((void *)newt->Displs, (void *)displs, displs_sz);
		}
	}

	/* 4.3. Block lengths */
	if ((kind == DTYPE_VECTOR) || (kind == DTYPE_HVECTOR)) {
		for (i = 0; i < count; i++)
			newt->BlkLen[i] = *blklen;
	} else
		memcpy(newt->BlkLen, blklen, items_sz);

	/* 5. Find the min and max displacements not specified through LB or UB */
	/* 5.1. Initial value for displacements */
	for (i = 0; i < count; i++) {
		if (((int) (newt->Types[i]) != MPI_P_LB)
				&& ((int) (newt->Types[i]) != MPI_P_UB)) {
			min_disp = max_disp = newt->Displs[i];
			break;
		}
	}
	min_disp = 0;

	found_lb = found_ub = 0;
	epsilon = 0; /* By now in this implementation the user must use MPI_P_UB */
	for (i = 0; i < count; i++) {

		if (((int) (newt->Types[i]) != MPI_P_LB)
				&& ((int) (newt->Types[i]) != MPI_P_UB)) {

			if (min_disp > newt->Displs[i])
				min_disp = newt->Displs[i];

			max_calc = newt->Displs[i]
					+ (newt->Types[i]->Extent * newt->BlkLen[i]) + epsilon;
			if (max_disp < max_calc)
				max_disp = max_calc;

		} else if ((int) (newt->Types[i]) == MPI_P_LB) {
			found_lb = 1;
		} else if ((int) (newt->Types[i]) == MPI_P_UB) {
			found_ub = 1;
		}

		if (found_lb && found_ub)
			break;
	}

	/* 6. If found MPI_P_LB or MPI_P_UB, set lb and ub fields */
	if (found_lb || found_ub) {

		for (i = 0; i < count; i++) {

			if ((int) (newt->Types[i]) == MPI_P_LB) {
				new_lb = newt->Displs[i];
			}

			else if ((int) (newt->Types[i]) == MPI_P_UB) {
				new_ub = newt->Displs[i];
			}

		}
	}

	if (!found_lb)
		new_lb = min_disp;
	if (!found_ub)
		new_ub = max_disp;

	/* 7. Set the number of primitive and derived datatypes fields, and the size in bytes */
	newt->Elements = 0;
	newt->Blocks = 0;
	newt->Size = 0;

	for (i = 0; i < count; i++) {

		if (((int) (newt->Types[i]) != MPI_P_LB)
				&& ((int) (newt->Types[i]) != MPI_P_UB)) {
			newt->Elements += (newt->Types[i]->Elements * newt->BlkLen[i]);
			newt->Blocks += newt->BlkLen[i];
			newt->Size += (newt->Types[i]->Size * newt->BlkLen[i]);
		}

	}

	/* 8. Set extent */
	newt->Lb = new_lb;
	newt->Ub = new_ub;
	newt->Extent = newt->Ub - newt->Lb;

	/* 9. Is it a contiguos data type?. The algorithm calculates the extent of each
	 subdatatype times the blocklen and compare it with the displacement of
	 these subdatatype */

	for (i = 0; i < count; i++) {
		if (((int) (newt->Types[i]) != MPI_P_LB)
				&& ((int) (newt->Types[i]) != MPI_P_UB)
				&& (!(newt->Types[i]->State & DTYPE_MEM_CONTIG)))
			break;
	}

	if (i == count) {

		for (i = 0; i < count; i++) {
			if (((int) (newt->Types[i]) != MPI_P_LB)
					&& ((int) (newt->Types[i]) != MPI_P_UB)) {
				min_disp = newt->Displs[i];
				break;
			}
		}

		j = 0;
		i = 0;
		while (i < count) {
			i++;
			if (i == count)
				break;

			if (((int) (newt->Types[j]) == MPI_P_LB)
					|| ((int) (newt->Types[j]) == MPI_P_UB)) {
				j++;
				continue;
			}

			max_disp = newt->Displs[i];
			if ((((int) (newt->Types[i]) == MPI_P_UB)
					|| ((max_disp - min_disp)
							== (newt->BlkLen[j] * newt->Types[j]->Extent)))
					&& ((newt->Types[j]->State & DTYPE_MEM_CONTIG)))
				min_disp = newt->Displs[i];

			else
				break;

			j++;

		}
		if (i == count)
			newt->State |= DTYPE_MEM_CONTIG;

	}

	/* 10. Update state */
//newt->State |= DTYPE_STRUCT;
	newt->State |= kind;

	/* 11. Update references to all datatypes used */
	for (i = 0; i < count; i++) {
		if (((int) (newt->Types[i]) != MPI_P_LB)
				&& ((int) (newt->Types[i]) != MPI_P_UB)
				&& (!(newt->State & DTYPE_BASIC)))
			newt->Types[i]->Refs++;
	}
	newt->Refs = 1;

	/* 12. Size of a packed data of this type */
	rec_packSize(newt, &newt->PackSize);

	/* 13. Return new datatype created */
	*datatypenew = newt;

#ifdef DEBUG_MODE
	printDatatype(newt);
#endif

	return DTP_E_OK;
}

/*
 *  dtypeFree():
 *   Free a datatype.
 */
int dtypeFree(DTypeTable *dtypetable, Mpi_P_Datatype_t *datatype) {

	int i;

	if (*datatype == NULL )
		return DTP_E_OK;
	if ((*datatype)->State & DTYPE_BASIC)
		return DTP_E_INTEGRITY;

	/* 1. Delete refs to all derived subtypes */
	for (i = 0; i < (*datatype)->Count; i++) {
		if (((int) ((*datatype)->Types[i]) == MPI_P_LB)
				|| ((int) ((*datatype)->Types[i]) == MPI_P_UB)
				|| ((*datatype)->Types[i]->State & DTYPE_BASIC))
			continue;

		(*datatype)->Types[i]->Refs--;

		/* 1.1. If a datatype has not references, try to eliminate it and decrease
		 references from its subtypes */
		if ((*datatype)->Types[i]->Refs == 0) {
			dtypeFree(dtypetable, &((*datatype)->Types[i]));
		}

	}

	/* 2. All subtypes references has been deleted, so is a type is not been referenced
	 by a supertype, free it */
	(*datatype)->Refs--;

	if ((*datatype)->Refs <= 0) {
		free((*datatype)->Types);
		free((*datatype)->BlkLen);
		free((*datatype)->Displs);
		(*datatype)->State = 0;
		objFree((Object_t )dtypetable->Dtypes, datatype, (*datatype)->Index);
	}

	*datatype = (Mpi_P_Datatype *) NULL;

	return DTP_E_OK;
}

/*
 *  dtypeCommit():
 *   Commit a datatype
 */
int dtypeCommit(Mpi_P_Datatype *datatype) {
	datatype->State |= DTYPE_COMMITED;
	return DTP_E_OK;
}

/*
 *  pack():
 */
int _pack(void *inbuf, int incount, Mpi_P_Datatype *datatype, void *outbuf,
		int outsize, int *position) {

	Mpi_P_Aint displ;
	int i;

	for (i = 0; i < incount; i++) {
		displ = (i * datatype->Extent);
		rec_pack(inbuf, outbuf, datatype, position, &displ);
	}

	return DTP_E_OK;
}

/*
 *  pack():
 */
int pack(void *inbuf, int incount, Mpi_P_Datatype *datatype, void *outbuf,
		int outsize, int *position) {

	Mpi_P_Aint displ;
	int i;

	for (i = 0; i < incount; i++) {
		displ = (Mpi_P_Aint) (i * datatype->Extent);
		rec_pack(inbuf, outbuf, datatype, position, (Mpi_P_Aint *) &displ);
	}

	return DTP_E_OK;
}

/*
 *  unpack():
 */
int unpack(void *inbuf, int insize, int *position, void *outbuf, int outcount,
		Mpi_P_Datatype *datatype) {

	Mpi_P_Aint displ;
	int i;

	for (i = 0; i < outcount; i++) {
		displ = (Mpi_P_Aint) (i * datatype->Extent);
		rec_unpack(inbuf, outbuf, datatype, position, (Mpi_P_Aint *) &displ);
	}

	return DTP_E_OK;
}

/*
 *  dtypeGetByAddr:
 *   Return Index of a datatype in the "object" structure by address. This index
 *   is stored in a field of the object (datatype), so return it
 */
int dtypeGetByAddr(DTypeTable *dtypetab, Mpi_P_Datatype *dtype) {
	return dtype->Index;
}

/*
 *  dtypeGetByIndex:
 *   Return the reference to an object (datatype) by its index in the
 *   "object" structure.
 */
Mpi_P_Datatype *dtypeGetByIndex(DTypeTable *dtypetab, int index) {

	Mpi_P_Datatype *dtype;

	if (index < BASIC_DATATYPES) {
		return (&BasicDTypeTable[index]);
	}

	objGet((Object_t )dtypetab->Dtypes, index, &dtype);

	return dtype;
}

/*-------------------------------------------------------/
 /    Implementation of private DEBUG interface           /
 /-------------------------------------------------------*/
#ifdef DEBUG_MODE

PRIVATE int printDatatypeTable (DTypeTable *dtypetable) {

	int i;
	Mpi_P_Datatype *dtype;

	/*
	 for(i = 0; i < dtypetable->Size; i++) {
	 dtype = &dtypetable->Dtypes[i];

	 if (!(dtype->State & DTYPE_ALLOCATED)) continue;

	 printDatatype(dtype);
	 }
	 */
	return 0;
}

PRIVATE int printDatatype (Mpi_P_Datatype *dtype) {

	int k;

	if (getRank() != 0) return 0;

	fprintf(stdout, "+++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	fprintf(stdout, " >>>>>> DATATYPE (Process %d) <<<<<<  (Addr 0x%x)\n", getRank(), dtype);

	fprintf(stdout, "\tState: 0x%x\n", dtype->State);
	fprintf(stdout, "\tSize: %d    Primitive Elements: %d    Elements: %d \n", dtype->Size, dtype->Elements, dtype->Blocks);
	fprintf(stdout, "\tCount: %d   \n", dtype->Count);
	fprintf(stdout, "\tExtent: %d\n", dtype->Extent);
	fprintf(stdout, "\tLb: %d\tUb: %d\n", dtype->Lb, dtype->Ub);
	fprintf(stdout, "\tSize packed: %d\n", dtype->PackSize);
	fprintf(stdout, "\tContiguous: %s\n", (dtype->State & DTYPE_MEM_CONTIG) ? "TRUE" : "FALSE");

	/*fprintf(stdout, "Types\t-> ");
	 for (k = 0; k < dtype->Count; k++)
	 fprintf(stdout, "0x%x  ", dtype->Types[k]);

	 fprintf(stdout, "\nDisplacements\t-> ");
	 for (k = 0; k < dtype->Count; k++)
	 fprintf(stdout, "0x%x ", dtype->Displs[k]);

	 fprintf(stdout, "\nBlklens\t-> ");
	 for (k = 0; k < dtype->Count; k++)
	 fprintf(stdout, "%d ", dtype->BlkLen[k]);
	 */
	fprintf(stdout, "\n--------------------------------------------------------\n");

}

#endif

