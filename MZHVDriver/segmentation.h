/**
 * @file segmentation.h
 * @brief Functions for reading segment selectors.
 * 
 * This file provides declarations for functions that read segment selectors defined
 * in segmentation.asm file. It also provides declarations for unions and structs
 * that can be used for type punning.
 */


#pragma once


#include <ntddk.h>


/**************************************************************************************************
* Type declarations
**************************************************************************************************/
/**
 * Structure for segment selector register
 */
#pragma warning(disable:4201)
#pragma pack(push, 1)
typedef union SEGMENTATION_SegmentSelector
{
  UINT16 bits;
  struct
  {
    UINT16 requestPrivilegeLevel : 2;
    UINT16 tableIndicator : 1;
    UINT16 index : 13;
  };
} SEGMENTATION_SegmentSelector;


/**
 * @brief Structure for segment descriptor table register
 */
typedef struct SEGMENTATION_DTR
{
  UINT16 limit;
  UINT64 base;
} VMCS_GDTR, SEGMENTATION_IDTR;


/**
 * @brief Structure for segment access rights byte
 */
typedef union SEGMENTATION_SegmentAccessRightsByte
{
  UINT8 bits;
  struct
  {
    UINT8 segmentType : 4;
    UINT8 descriptorType : 1;
    UINT8 descriptorPrivilegeLevel : 2;
    UINT8 segmentPresent : 1;
  };
} SEGMENTATION_SegmentAccessRightsByte;


/**
 * @brief Structure for VMCS segment access rights
 */
typedef union SEGMENTATION_VMCSSegmentAccessRights
{
  UINT32 bits;
  struct
  {
    SEGMENTATION_SegmentAccessRightsByte segmentAccessRightsByte;
    UINT8 _pad1 : 4;
    UINT8 segmentFlags : 4;
    UINT16 segmentUnusable : 1;
    UINT16 _pad2 : 15;
  };
} SEGMENTATION_VMCSSegmentAccessRights;


/**
 * @brief Structure for segment descriptor
 */
typedef union SEGMENTATION_SegmentDescriptor
{
  UINT64 bits;
  struct
  {
    UINT16 segmentLimit1;
    UINT16 segmentBaseAddress1;
    UINT8 segmentBaseAddress2;
    SEGMENTATION_SegmentAccessRightsByte segmentAccessRightsByte;
    UINT8 segmentLimit2 : 4;
    UINT8 segmentFlags : 4;
    UINT8 segmentBaseAddress3;
  };
} SEGMENTATION_SegmentDescriptor;


/**
 * @brief Structure for system segment descriptor
 */
typedef union SEGMENTATION_SystemSegmentDescriptor
{
  struct
  {
    UINT64 lowerBits;
    UINT64 upperBits;
  };
  struct
  {
    SEGMENTATION_SegmentDescriptor segmentDescriptor;
    UINT32 segmentBaseAddress4;
    UINT32 _pad1;
  };
} SEGMENTATION_SystemSegmentDescriptor;


/**
 * @brief Structure for segment base, used for type punning
 */
typedef union SEGMENTATION_SegmentBase
{
  UINT64 bits;
  struct
  {
    UINT16 address1;
    UINT8 address2;
    UINT8 address3;
    UINT32 address4;
  };
} SEGMENTATION_SegmentBase;
#pragma pack(pop)
#pragma warning(default:4201)


/**************************************************************************************************
* Global function declarations
**************************************************************************************************/
/**
 * @brief Reads segment selector from the CS register.
 * 
 * @return Segment selector
 */
SEGMENTATION_SegmentSelector SEGMENTATION_readCs(VOID);


/**
 * @brief Reads segment selector from SS register.
 * 
 * @return Segment selector
 */
SEGMENTATION_SegmentSelector SEGMENTATION_readSs(VOID);


/**
 * @brief Reads segment selector from DS register.
 *
 * @return Segment selector
 */
SEGMENTATION_SegmentSelector SEGMENTATION_readDs(VOID);


/**
 * @brief Reads segment selector from ES register.
 *
 * @return Segment selector
 */
SEGMENTATION_SegmentSelector SEGMENTATION_readEs(VOID);


/**
 * @brief Reads segment selector from FS register.
 *
 * @return Segment selector
 */
SEGMENTATION_SegmentSelector SEGMENTATION_readFs(VOID);


/**
 * @brief Reads segment selector from GS register.
 *
 * @return Segment selector
 */
SEGMENTATION_SegmentSelector SEGMENTATION_readGs(VOID);


/**
 * @brief Reads segment selector from LDTR register.
 *
 * @return Segment selector
 */
SEGMENTATION_SegmentSelector SEGMENTATION_readLdtr(VOID);


/**
 * @brief Reads segment selector from TR register.
 *
 * @return Segment selector
 */
SEGMENTATION_SegmentSelector SEGMENTATION_readTr(VOID);
