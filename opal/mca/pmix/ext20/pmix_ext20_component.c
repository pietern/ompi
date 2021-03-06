/*
 * Copyright (c) 2014-2016 Intel, Inc.  All rights reserved.
 * Copyright (c) 2014-2015 Research Organization for Information Science
 *                         and Technology (RIST). All rights reserved.
 * Copyright (c) 2016 Cisco Systems, Inc.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 *
 * These symbols are in a file by themselves to provide nice linker
 * semantics.  Since linkers generally pull in symbols by object
 * files, keeping these symbols as the only symbols in this file
 * prevents utility programs such as "ompi_info" from having to import
 * entire components just to query their version and parameters.
 */

#include "opal_config.h"

#include "opal/constants.h"
#include "opal/class/opal_list.h"
#include "opal/util/proc.h"
#include "opal/mca/pmix/pmix.h"
#include "pmix_ext20.h"

/*
 * Public string showing the pmix external component version number
 */
const char *opal_pmix_ext20_component_version_string =
    "OPAL external pmix2.0 MCA component version " OPAL_VERSION;

/*
 * Local function
 */
static int external_open(void);
static int external_close(void);
static int external_component_query(mca_base_module_t **module, int *priority);
static int external_register(void);


/*
 * Instantiate the public struct with all of our public information
 * and pointers to our public functions in it
 */

mca_pmix_ext20_component_t mca_pmix_ext20_component = {
    {
    /* First, the mca_component_t struct containing meta information
       about the component itself */

        .base_version = {
        /* Indicate that we are a pmix v1.1.0 component (which also
           implies a specific MCA version) */

            OPAL_PMIX_BASE_VERSION_2_0_0,

        /* Component name and version */

            .mca_component_name = "ext20",
            MCA_BASE_MAKE_VERSION(component, OPAL_MAJOR_VERSION, OPAL_MINOR_VERSION,
                                  OPAL_RELEASE_VERSION),

        /* Component open and close functions */

            .mca_open_component = external_open,
            .mca_close_component = external_close,
            .mca_query_component = external_component_query,
            .mca_register_component_params = external_register,
        },
        /* Next the MCA v1.0.0 component meta data */
        .base_data = {
        /* The component is checkpoint ready */
            MCA_BASE_METADATA_PARAM_CHECKPOINT
        }
    },
    .native_launch = false
};

static int external_register(void)
{
    mca_pmix_ext20_component.cache_size = 256;
    mca_base_component_var_register(&mca_pmix_ext20_component.super.base_version,
                                    "cache_size", "Size of the ring buffer cache for events",
                                    MCA_BASE_VAR_TYPE_INT, NULL, 0, 0, OPAL_INFO_LVL_5,
                                    MCA_BASE_VAR_SCOPE_CONSTANT,
                                    &mca_pmix_ext20_component.cache_size);

    return OPAL_SUCCESS;
}


static int external_open(void)
{
    mca_pmix_ext20_component.evindex = 0;
    OBJ_CONSTRUCT(&mca_pmix_ext20_component.jobids, opal_list_t);
    OBJ_CONSTRUCT(&mca_pmix_ext20_component.single_events, opal_list_t);
    OBJ_CONSTRUCT(&mca_pmix_ext20_component.multi_events, opal_list_t);
    OBJ_CONSTRUCT(&mca_pmix_ext20_component.default_events, opal_list_t);
    OBJ_CONSTRUCT(&mca_pmix_ext20_component.cache, opal_list_t);

    return OPAL_SUCCESS;
}

static int external_close(void)
{
    OPAL_LIST_DESTRUCT(&mca_pmix_ext20_component.jobids);
    OPAL_LIST_DESTRUCT(&mca_pmix_ext20_component.single_events);
    OPAL_LIST_DESTRUCT(&mca_pmix_ext20_component.multi_events);
    OPAL_LIST_DESTRUCT(&mca_pmix_ext20_component.default_events);
    OPAL_LIST_DESTRUCT(&mca_pmix_ext20_component.cache);
    return OPAL_SUCCESS;
}


static int external_component_query(mca_base_module_t **module, int *priority)
{
    char *t, *id;

    /* see if a PMIx server is present */
    if (NULL != (t = getenv("PMIX_SERVER_URI")) ||
        NULL != (id = getenv("PMIX_ID"))) {
        /* if PMIx is present, then we are a client and need to use it */
        *priority = 100;
    } else {
        /* we could be a server, so we still need to be considered */
        *priority = 5;
    }
    *module = (mca_base_module_t *)&opal_pmix_ext20_module;
    return OPAL_SUCCESS;
}
