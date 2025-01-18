/**
 * @file context.c
 * @brief This file implements logic for context management.
 * 
 * Context is a global structure that contains processor information.
 * It is used to store VMXON, VMCS regions, MSR bitmaps, mappings data, etc.
 */


#include "context.h"
#include "memory.h"


/**************************************************************************************************
* Local globals definitions
**************************************************************************************************/
/**
 * @brief Context of all logical cores.
 */
static Context_Context* context = NULL;


/**************************************************************************************************
* Global function definitions
**************************************************************************************************/
/**
 * @brief Initializes context.
 *
 * @return STATUS_SUCCESS when successful, STATUS_UNSUCCESSFUL otherwise.
 */
NTSTATUS Context_init(VOID)
{
  const UINT64 noOfLogicalCores = KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
  context = Memory_allocate(
    sizeof(*context) + noOfLogicalCores * sizeof(Context_LogicalCore),
    TRUE);
  if (context == NULL)
  {
    return STATUS_UNSUCCESSFUL;
  }

  context->noOfLogicalCores = noOfLogicalCores;

  return STATUS_SUCCESS;
}


/**
 * @brief Getter for context.
 * 
 * @return Pointer to context.
 */
Context_Context* Context_getContext(VOID)
{
  return context;
}


/**
 * @brief Getter for this core's context.
 * 
 * @return Pointer to context of current logical core.
 */
Context_LogicalCore* Context_getLogicalCore(VOID)
{
  if (context == NULL)
  {
    return NULL;
  }

  return &context->logicalCores[KeGetCurrentProcessorNumber()];
}


/**
 * @brief Destroys the context.
 * 
 * @return VOID
 */
VOID Context_destroy(VOID)
{
  Memory_free(context);
  context = NULL;
}
