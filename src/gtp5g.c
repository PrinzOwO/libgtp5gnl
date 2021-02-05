/* External interface functions of the library */

/* (C) 2014 by sysmocom - s.f.m.c. GmbH
 * (C) 2016 by Pablo Neira Ayuso <pablo@gnumonks.org>
 * Author: Yao-Wen Chang <yaowenowo@gmail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <regex.h>
#include <limits.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <linux/gtp5g.h>
#include <libgtp5gnl/gtp5g.h>

#include "internal.h"
#include "tools.h"

#define GTP5G_STRUCT_FUNC_NAME(x) gtp5g_ ##x## _alloc
#define gtp5g_struct_alloc_exp(__func_name, __ret_type) \
__ret_type *GTP5G_STRUCT_FUNC_NAME(__func_name)(void) \
{ \
    __ret_type *ptr; \
    ptr = calloc(1, sizeof(__ret_type)); \
    if (!ptr) \
        return NULL; \
    return ptr; \
} \
EXPORT_SYMBOL(GTP5G_STRUCT_FUNC_NAME(__func_name))

#define gtp5g_struct_alloc_no_exp(__func_name, __ret_type) \
static inline __ret_type *GTP5G_STRUCT_FUNC_NAME(__func_name)(void) \
{ \
    __ret_type *ptr; \
    ptr = calloc(1, sizeof(__ret_type)); \
    if (!ptr) \
        return NULL; \
    return ptr; \
}

struct gtp5g_dev *gtp5g_dev_alloc(void)
{
	struct gtp5g_dev *dev;

	dev = calloc(1, sizeof(struct gtp5g_dev));
	if (!dev)
		return NULL;

	dev->ifns = -1;
	return dev;
}
EXPORT_SYMBOL(gtp5g_dev_alloc);

/**
 * PDR
 * */
gtp5g_struct_alloc_exp(pdr, struct gtp5g_pdr);
gtp5g_struct_alloc_no_exp(pdi, struct gtp5g_pdi);
gtp5g_struct_alloc_no_exp(precedence, uint32_t);
gtp5g_struct_alloc_no_exp(pdr_outer_header_removal, uint8_t);
gtp5g_struct_alloc_no_exp(pdr_far_id, uint32_t);
gtp5g_struct_alloc_no_exp(pdr_qer_id, uint32_t);

/* Not in 3GPP spec, just used for routing */
gtp5g_struct_alloc_no_exp(role_addr_ipv4, struct in_addr);

/* Not in 3GPP spec, just used for buffering */
static inline char *gtp5g_unix_sock_path_alloc(void)
{
	char *unix_sock_path;

	unix_sock_path = calloc(108, sizeof(char)); // sun_path[108]
	if (!unix_sock_path)
		return NULL;

	return unix_sock_path;
}

/* Nest in PDI */ 
gtp5g_struct_alloc_no_exp(pdi_ue_addr_ipv4, struct in_addr);
gtp5g_struct_alloc_no_exp(pdi_local_f_teid, struct local_f_teid);
gtp5g_struct_alloc_no_exp(pdi_sdf_filter, struct sdf_filter);

/**
 * SDF 
 * */
/* Nest in SDF Filter */
gtp5g_struct_alloc_no_exp(sdf_filter_description, struct ip_filter_rule);
gtp5g_struct_alloc_no_exp(sdf_filter_tos_traffic_class, uint16_t);
gtp5g_struct_alloc_no_exp(sdf_filter_security_param_idx, uint32_t);
gtp5g_struct_alloc_no_exp(sdf_filter_flow_label, uint32_t);
gtp5g_struct_alloc_no_exp(sdf_filter_id, uint32_t);

/**
 * FAR
 * */
gtp5g_struct_alloc_exp(far, struct gtp5g_far);
gtp5g_struct_alloc_no_exp(forwarding_parameter, struct gtp5g_forwarding_parameter);

/* Nest in Forwarding Parameter */
gtp5g_struct_alloc_no_exp(outer_header_creation, struct gtp5g_outer_header_creation);
gtp5g_struct_alloc_no_exp(forwarding_policy, struct gtp5g_forwarding_policy);

/**
 * QER
 * */
gtp5g_struct_alloc_exp(qer, struct gtp5g_qer);

void gtp5g_dev_free(struct gtp5g_dev *dev)
{
    free(dev);
}
EXPORT_SYMBOL(gtp5g_dev_free);

static void gtp5g_sdf_filter_description_free(struct ip_filter_rule *rule)
{
    if (rule->sport_list)
        free(rule->sport_list);
    
    if(rule->dport_list)
        free(rule->dport_list);

    free(rule);
}

static void gtp5g_sdf_filter_free(struct sdf_filter *sdf)
{
    if (sdf->rule)
        gtp5g_sdf_filter_description_free(sdf->rule);

    if (sdf->tos_traffic_class)
        free(sdf->tos_traffic_class);

    if (sdf->security_param_idx)
        free(sdf->security_param_idx);

    if (sdf->flow_label)
        free(sdf->flow_label);

    if (sdf->bi_id)
        free(sdf->bi_id);
}

static void gtp5g_pdi_free(struct gtp5g_pdi *pdi)
{
    if (pdi->ue_addr_ipv4)
        free(pdi->ue_addr_ipv4);

    if (pdi->f_teid)
        free(pdi->f_teid);

    if (pdi->sdf)
        gtp5g_sdf_filter_free(pdi->sdf);

    free(pdi);
}

void gtp5g_pdr_free(struct gtp5g_pdr *pdr)
{
    if (pdr->precedence)
        free(pdr->precedence);
    
    if (pdr->pdi)
        gtp5g_pdi_free(pdr->pdi);

    if (pdr->outer_hdr_removal)
        free(pdr->outer_hdr_removal);

    if (pdr->far_id)
        free(pdr->far_id);

    if (pdr->qer_id)
        free(pdr->qer_id);

    /* Not in 3GPP spec, just used for routing */
    if (pdr->role_addr_ipv4)
        free(pdr->role_addr_ipv4);

    if (pdr->unix_sock_path)
        free(pdr->unix_sock_path);

    free(pdr);
}
EXPORT_SYMBOL(gtp5g_pdr_free);

static void gtp5g_forwarding_parameter_free(struct gtp5g_forwarding_parameter *fwd_param)
{
    if (fwd_param->hdr_creation)
        free(fwd_param->hdr_creation);

    free(fwd_param);
}

void gtp5g_far_free(struct gtp5g_far *far)
{
    if (far->fwd_param)
        gtp5g_forwarding_parameter_free(far->fwd_param);

    if (far->related_pdr_list)
        free(far->related_pdr_list);

    free(far);
}
EXPORT_SYMBOL(gtp5g_far_free);

/* Not in 3GPP spec, just used for routing */
static inline void role_addr_ipv4_may_alloc(struct gtp5g_pdr *pdr)
{
    if (!pdr->role_addr_ipv4)
        pdr->role_addr_ipv4 = gtp5g_role_addr_ipv4_alloc();
}

/* Not in 3GPP spec, just used for buffering */
static inline void unix_sock_path_may_alloc(struct gtp5g_pdr *pdr)
{
    if (!pdr->unix_sock_path)
        pdr->unix_sock_path = gtp5g_unix_sock_path_alloc();
}

static inline void precedence_may_alloc(struct gtp5g_pdr *pdr)
{
    if (!pdr->precedence)
        pdr->precedence = gtp5g_precedence_alloc();
}

static inline void pdi_may_alloc(struct gtp5g_pdr *pdr)
{
    if (!pdr->pdi)
        pdr->pdi = gtp5g_pdi_alloc();
}

static inline void outer_hdr_removal_may_alloc(struct gtp5g_pdr *pdr)
{
    if (!pdr->outer_hdr_removal)
        pdr->outer_hdr_removal = gtp5g_pdr_outer_header_removal_alloc();
}

static inline void far_id_may_alloc(struct gtp5g_pdr *pdr)
{
    if (!pdr->far_id)
        pdr->far_id = gtp5g_pdr_far_id_alloc();
}

/* NOTE: gtp5g_struct_alloc_no_exp() */
static inline void qer_id_may_alloc(struct gtp5g_pdr *pdr)
{
    if (!pdr->qer_id)
        pdr->qer_id = gtp5g_pdr_qer_id_alloc();
}

static inline void ue_addr_ipv4_may_alloc(struct gtp5g_pdr *pdr)
{
    pdi_may_alloc(pdr);
    if(!pdr->pdi->ue_addr_ipv4)
        pdr->pdi->ue_addr_ipv4 = gtp5g_pdi_ue_addr_ipv4_alloc();
}

static inline void local_f_teid_may_alloc(struct gtp5g_pdr *pdr)
{
    pdi_may_alloc(pdr);
    if (!pdr->pdi->f_teid)
        pdr->pdi->f_teid = gtp5g_pdi_local_f_teid_alloc();
}

static inline void sdf_filter_may_alloc(struct gtp5g_pdr *pdr)
{
    pdi_may_alloc(pdr);
    if (!pdr->pdi->sdf)
        pdr->pdi->sdf = gtp5g_pdi_sdf_filter_alloc();
}

static inline void sdf_filter_description_may_alloc(struct gtp5g_pdr *pdr)
{
    sdf_filter_may_alloc(pdr);
    if (!pdr->pdi->sdf->rule)
        pdr->pdi->sdf->rule = gtp5g_sdf_filter_description_alloc();
}

static inline void sdf_filter_tos_traffic_class_may_alloc(struct gtp5g_pdr *pdr)
{
    sdf_filter_may_alloc(pdr);
    if (!pdr->pdi->sdf->tos_traffic_class)
        pdr->pdi->sdf->tos_traffic_class = gtp5g_sdf_filter_tos_traffic_class_alloc();
}

static inline void sdf_filter_security_param_idx_may_alloc(struct gtp5g_pdr *pdr)
{
    sdf_filter_may_alloc(pdr);
    if (!pdr->pdi->sdf->security_param_idx)
        pdr->pdi->sdf->security_param_idx = gtp5g_sdf_filter_security_param_idx_alloc();
}

static inline void sdf_filter_flow_label_may_alloc(struct gtp5g_pdr *pdr)
{
    sdf_filter_may_alloc(pdr);
    if (!pdr->pdi->sdf->flow_label)
        pdr->pdi->sdf->flow_label = gtp5g_sdf_filter_flow_label_alloc();
}

static inline void sdf_filter_id_may_alloc(struct gtp5g_pdr *pdr)
{
    sdf_filter_may_alloc(pdr);
    if (!pdr->pdi->sdf->bi_id)
        pdr->pdi->sdf->bi_id = gtp5g_sdf_filter_id_alloc();
}

static inline void fwd_param_may_alloc(struct gtp5g_far *far)
{
    if (!far->fwd_param)
        far->fwd_param = gtp5g_forwarding_parameter_alloc();
}

static inline void outer_hdr_creation_may_alloc(struct gtp5g_far *far)
{
    fwd_param_may_alloc(far);
    if (!far->fwd_param->hdr_creation)
        far->fwd_param->hdr_creation = gtp5g_outer_header_creation_alloc();
}

static inline void fwd_policy_may_alloc(struct gtp5g_far *far)
{
    fwd_param_may_alloc(far);
    if (!far->fwd_param->fwd_policy)
        far->fwd_param->fwd_policy = gtp5g_forwarding_policy_alloc();
}


void gtp5g_dev_set_ifns(struct gtp5g_dev *dev, int ifns)
{
    dev->ifns = ifns;
}
EXPORT_SYMBOL(gtp5g_dev_set_ifns);

void gtp5g_dev_set_ifidx(struct gtp5g_dev *dev, uint32_t ifidx)
{
    dev->ifidx = ifidx;
}
EXPORT_SYMBOL(gtp5g_dev_set_ifidx);

void gtp5g_pdr_set_id(struct gtp5g_pdr *pdr, uint16_t id)
{
    pdr->id = id;
}
EXPORT_SYMBOL(gtp5g_pdr_set_id);

void gtp5g_pdr_set_precedence(struct gtp5g_pdr *pdr, uint32_t precedence)
{
    precedence_may_alloc(pdr);
    *pdr->precedence = precedence;
}
EXPORT_SYMBOL(gtp5g_pdr_set_precedence);

void gtp5g_pdr_set_far_id(struct gtp5g_pdr *pdr, uint32_t far_id) {
    far_id_may_alloc(pdr);
    *pdr->far_id = far_id;
}
EXPORT_SYMBOL(gtp5g_pdr_set_far_id);

void gtp5g_pdr_set_outer_header_removal(struct gtp5g_pdr *pdr, uint8_t outer_hdr_removal)
{
    outer_hdr_removal_may_alloc(pdr);
    *pdr->outer_hdr_removal = outer_hdr_removal;
}
EXPORT_SYMBOL(gtp5g_pdr_set_outer_header_removal);

/* Not in 3GPP spec, just used for routing */
void gtp5g_pdr_set_role_addr_ipv4(struct gtp5g_pdr *pdr, struct in_addr *role_addr_ipv4)
{
    role_addr_ipv4_may_alloc(pdr);
    memcpy(pdr->role_addr_ipv4, role_addr_ipv4, sizeof(struct in_addr));
}
EXPORT_SYMBOL(gtp5g_pdr_set_role_addr_ipv4);

/* Not in 3GPP spec, just used for buffering */
void gtp5g_pdr_set_unix_sock_path(struct gtp5g_pdr *pdr, const char *unix_sock_path)
{
    unix_sock_path_may_alloc(pdr);
    strcpy(pdr->unix_sock_path, unix_sock_path);
}
EXPORT_SYMBOL(gtp5g_pdr_set_unix_sock_path);

void gtp5g_pdr_set_ue_addr_ipv4(struct gtp5g_pdr *pdr, struct in_addr *ue_addr_ipv4)
{
    ue_addr_ipv4_may_alloc(pdr);
    memcpy(pdr->pdi->ue_addr_ipv4, ue_addr_ipv4, sizeof(struct in_addr));
}
EXPORT_SYMBOL(gtp5g_pdr_set_ue_addr_ipv4);

void gtp5g_pdr_set_local_f_teid(struct gtp5g_pdr *pdr, uint32_t teid, struct in_addr *gtpu_addr_ipv4)
{
    local_f_teid_may_alloc(pdr);
    pdr->pdi->f_teid->teid = teid;
    memcpy(&pdr->pdi->f_teid->gtpu_addr_ipv4, gtpu_addr_ipv4, sizeof(struct in_addr));
}
EXPORT_SYMBOL(gtp5g_pdr_set_local_f_teid);

void gtp5g_pdr_set_sdf_filter_description(struct gtp5g_pdr *pdr, const char *rule_str)
{
    sdf_filter_description_may_alloc(pdr);
    struct ip_filter_rule *rule = pdr->pdi->sdf->rule;

    char reg_act[] = "(permit)";
    char reg_direction[] = "(in|out)";
    char reg_proto[] = "(ip|[0-9]{1,3}})";
    char reg_src_ip_mask[] = "(any|assigned|[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}(/[0-9]{1,5})?)";
    char reg_dest_ip_mask[] = "(any|assigned|[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}(/[0-9]{1,5})?)";
    char reg_port[] = "([ ][0-9]{1,5}([,-][0-9]{1,5})*)?";

    char reg[0xfff];
    sprintf(reg, "^%s %s %s from %s%s to %s%s$", reg_act, reg_direction, reg_proto,
                                                 reg_src_ip_mask, reg_port,
                                                 reg_dest_ip_mask, reg_port);

    regex_t preg;
    regmatch_t pmatch[0x10];
    int nmatch = sizeof(pmatch) / sizeof(regmatch_t);
    int cflags = REG_EXTENDED | REG_ICASE;

    if (regcomp(&preg, reg, cflags) != 0) {
        perror("Regex string for SDF filter description format error");
        goto err;
    }
    if (regexec(&preg, rule_str, nmatch, pmatch, 0) != 0) {
        perror("SDF filter description format error");
        goto err;
    }

    int len;
    char buf[0xff];

    // Get Action
    len = pmatch[1].rm_eo - pmatch[1].rm_so;
    strncpy(buf, rule_str + pmatch[1].rm_so, len); buf[len] = '\0';
    if (strcmp(buf, "permit") == 0) {
        rule->action = GTP5G_SDF_FILTER_PERMIT;
    }
    else {
        perror("SDF filter description action not support");
        goto err;
    }

    // Get Direction
    len = pmatch[2].rm_eo - pmatch[2].rm_so;
    strncpy(buf, rule_str + pmatch[2].rm_so, len); buf[len] = '\0';
    if (strcmp(buf, "in") == 0)
        rule->direction = GTP5G_SDF_FILTER_IN;
    else if (strcmp(buf, "out") == 0)
        rule->direction = GTP5G_SDF_FILTER_OUT;
    else {
        perror("SDF filter description direction not support");
        goto err;
    }

    // Get Protocol
    len = pmatch[3].rm_eo - pmatch[3].rm_so;
    strncpy(buf, rule_str + pmatch[3].rm_so, len); buf[len] = '\0';
    if (strcmp(buf, "ip") == 0)
        rule->proto = 0xff;
    else {
        int tmp = atoi(buf);
        if (tmp > 0xff) {
            perror("SDF filter description protocol not support");
            goto err;
        }
        rule->proto = tmp;
    }

    // Get SRC Mask
    len = pmatch[5].rm_eo - pmatch[5].rm_so;
    if (len) {
        strncpy(buf, rule_str + pmatch[5].rm_so + 1, len - 1); buf[len - 1] = '\0';
        int smask = atoi(buf);
        if (smask > 32) {
            perror("SDF filter description SRC mask is invalid");
            goto err;
        }
        rule->smask.s_addr = decimal_to_netmask(smask);
    }
    else
        rule->smask.s_addr = UINT_MAX;

    // Get SRC IP
    len = pmatch[4].rm_eo - pmatch[4].rm_so - len;
    strncpy(buf, rule_str + pmatch[4].rm_so, len); buf[len] = '\0';
    if (strcmp(buf, "any") == 0) {
        inet_pton(AF_INET, "0.0.0.0", &rule->src);
        rule->smask.s_addr = 0;
    }
    else if (strcmp(buf, "assigned") == 0) {
        perror("SDF filter description dest ip do NOT support assigned yet");
        goto err;
    }
    else if(inet_pton(AF_INET, buf, &rule->src) != 1) {
        perror("SDF filter description src ip is invalid");
        goto err;
    }

    // Get SRC Port
    len = pmatch[6].rm_eo - pmatch[6].rm_so;
    if (len) {
        strncpy(buf, rule_str + pmatch[6].rm_so + 1, len - 1); buf[len - 1] = '\0';
        rule->sport_list = port_list_create(buf);
    }
    else
        rule->sport_list = NULL;

    // Get Dest Mask
    len = pmatch[9].rm_eo - pmatch[9].rm_so;
    if (len) {
        strncpy(buf, rule_str + pmatch[9].rm_so + 1, len - 1); buf[len - 1] = '\0';
        int dmask = atoi(buf);
        if (dmask > 32) {
            perror("SDF filter description Dest mask is invalid");
            goto err;
        }
        rule->dmask.s_addr = decimal_to_netmask(dmask);
    }
    else
        rule->dmask.s_addr = UINT_MAX;

    // Get Dest IP
    len = pmatch[8].rm_eo - pmatch[8].rm_so - len;
    strncpy(buf, rule_str + pmatch[8].rm_so, len); buf[len] = '\0';
    if (strcmp(buf, "any") == 0) {
        inet_pton(AF_INET, "0.0.0.0", &rule->dest);
        rule->dmask.s_addr = 0;
    }
    else if (strcmp(buf, "assigned") == 0) {
        perror("SDF filter description dest ip do NOT support assigned yet");
        goto err;
    }
    else if(inet_pton(AF_INET, buf, &rule->dest) != 1) {
        perror("SDF filter description dest ip is invalid");
        goto err;
    }

    // Get Dest Port
    len = pmatch[10].rm_eo - pmatch[10].rm_so;
    if (len) {
        strncpy(buf, rule_str + pmatch[10].rm_so + 1, len - 1); buf[len - 1] = '\0';
        rule->dport_list = port_list_create(buf);
    }
    else
        rule->dport_list = NULL;

    return;
err:
    gtp5g_sdf_filter_free(pdr->pdi->sdf);
    return;
}
EXPORT_SYMBOL(gtp5g_pdr_set_sdf_filter_description);

void gtp5g_pdr_set_tos_traffic_class(struct gtp5g_pdr *pdr, uint16_t tos_traffic_class)
{
    sdf_filter_tos_traffic_class_may_alloc(pdr);
    *pdr->pdi->sdf->tos_traffic_class = tos_traffic_class;
}
EXPORT_SYMBOL(gtp5g_pdr_set_tos_traffic_class);

void gtp5g_pdr_set_security_param_idx(struct gtp5g_pdr *pdr, uint32_t security_param_idx)
{
    sdf_filter_security_param_idx_may_alloc(pdr);
    *pdr->pdi->sdf->security_param_idx = security_param_idx;
}
EXPORT_SYMBOL(gtp5g_pdr_set_security_param_idx);

void gtp5g_pdr_set_flow_label(struct gtp5g_pdr *pdr, uint32_t flow_label)
{
    sdf_filter_flow_label_may_alloc(pdr);
    *pdr->pdi->sdf->flow_label = flow_label;
}
EXPORT_SYMBOL(gtp5g_pdr_set_flow_label);

void gtp5g_pdr_set_sdf_filter_id(struct gtp5g_pdr *pdr, uint32_t id)
{
    sdf_filter_id_may_alloc(pdr);
    *pdr->pdi->sdf->bi_id = id;
}
EXPORT_SYMBOL(gtp5g_pdr_set_sdf_filter_id);

uint16_t *gtp5g_pdr_get_id(struct gtp5g_pdr *pdr)
{
    return &pdr->id;
}
EXPORT_SYMBOL(gtp5g_pdr_get_id);

uint32_t *gtp5g_pdr_get_precedence(struct gtp5g_pdr *pdr)
{
    return pdr->precedence;
}
EXPORT_SYMBOL(gtp5g_pdr_get_precedence);

uint32_t *gtp5g_pdr_get_far_id(struct gtp5g_pdr *pdr)
{
    return pdr->far_id;
}
EXPORT_SYMBOL(gtp5g_pdr_get_far_id);

uint32_t *gtp5g_pdr_get_qer_id(struct gtp5g_pdr *pdr)
{
    return pdr->qer_id;
}
EXPORT_SYMBOL(gtp5g_pdr_get_qer_id);

uint8_t *gtp5g_pdr_get_outer_header_removal(struct gtp5g_pdr *pdr)
{
    return pdr->outer_hdr_removal;
}
EXPORT_SYMBOL(gtp5g_pdr_get_outer_header_removal);

struct in_addr *gtp5g_pdr_get_ue_addr_ipv4(struct gtp5g_pdr *pdr)
{
    struct gtp5g_pdi *pdi = pdr->pdi;
    return (pdi ? pdi->ue_addr_ipv4 : NULL);
}
EXPORT_SYMBOL(gtp5g_pdr_get_ue_addr_ipv4);

uint32_t *gtp5g_pdr_get_local_f_teid_teid(struct gtp5g_pdr *pdr)
{
    struct local_f_teid *f_teid = (pdr->pdi ? pdr->pdi->f_teid : NULL);
    return (f_teid ? &f_teid->teid : NULL);
}
EXPORT_SYMBOL(gtp5g_pdr_get_local_f_teid_teid);

struct in_addr *gtp5g_pdr_get_local_f_teid_gtpu_addr_ipv4(struct gtp5g_pdr *pdr)
{
    struct local_f_teid *f_teid = (pdr->pdi ? pdr->pdi->f_teid : NULL);
    return (f_teid ? &f_teid->gtpu_addr_ipv4 : NULL);
}
EXPORT_SYMBOL(gtp5g_pdr_get_local_f_teid_gtpu_addr_ipv4);

void gtp5g_far_set_id(struct gtp5g_far *far, uint32_t id)
{
    far->id = id;
}
EXPORT_SYMBOL(gtp5g_far_set_id);

void gtp5g_far_set_apply_action(struct gtp5g_far *far, uint8_t apply_action)
{
    far->apply_action = apply_action;
}
EXPORT_SYMBOL(gtp5g_far_set_apply_action);

void gtp5g_far_set_outer_header_creation(struct gtp5g_far *far,
                                         uint16_t desp, uint32_t teid,
                                         struct in_addr *peer_addr_ipv4, uint16_t port)
{
    outer_hdr_creation_may_alloc(far);
    struct gtp5g_outer_header_creation *hdr_creation = far->fwd_param->hdr_creation;
    hdr_creation->desp = desp;
    hdr_creation->teid = teid;
    memcpy(&hdr_creation->peer_addr_ipv4, peer_addr_ipv4, sizeof(hdr_creation->peer_addr_ipv4));
    hdr_creation->port = port;
}
EXPORT_SYMBOL(gtp5g_far_set_outer_header_creation);

void gtp5g_far_set_fwd_policy(struct gtp5g_far *far, char *str)
{
    if (strlen(str) > MAX_LEN_OF_FORWARDING_POLICY_IDENTIFIER) {
        perror("Forwarding parameters is too long");
        return;
    }

    fwd_policy_may_alloc(far);
    struct gtp5g_forwarding_policy *fwd_policy = far->fwd_param->fwd_policy;
    fwd_policy->len = strlen(str);
    strcpy(fwd_policy->identifier, str);
}
EXPORT_SYMBOL(gtp5g_far_set_fwd_policy);

uint32_t *gtp5g_far_get_id(struct gtp5g_far *far)
{
    return &far->id;
}
EXPORT_SYMBOL(gtp5g_far_get_id);

uint8_t *gtp5g_far_get_apply_action(struct gtp5g_far *far)
{
    return &far->apply_action;
}
EXPORT_SYMBOL(gtp5g_far_get_apply_action);

uint16_t *gtp5g_far_get_outer_header_creation_description(struct gtp5g_far *far)
{
    struct gtp5g_outer_header_creation *hdr_creation = (far->fwd_param ? far->fwd_param->hdr_creation : NULL);
    return (hdr_creation ? &hdr_creation->desp :NULL);
}
EXPORT_SYMBOL(gtp5g_far_get_outer_header_creation_description);

uint32_t *gtp5g_far_get_outer_header_creation_teid(struct gtp5g_far *far)
{
    struct gtp5g_outer_header_creation *hdr_creation = (far->fwd_param ? far->fwd_param->hdr_creation : NULL);
    return (hdr_creation ? &hdr_creation->teid :NULL);
}
EXPORT_SYMBOL(gtp5g_far_get_outer_header_creation_teid);

struct in_addr *gtp5g_far_get_outer_header_creation_peer_addr_ipv4(struct gtp5g_far *far)
{
    struct gtp5g_outer_header_creation *hdr_creation = (far->fwd_param ? far->fwd_param->hdr_creation : NULL);
    return (hdr_creation ? &hdr_creation->peer_addr_ipv4 : NULL);
}
EXPORT_SYMBOL(gtp5g_far_get_outer_header_creation_peer_addr_ipv4);

uint16_t *gtp5g_far_get_outer_header_creation_port(struct gtp5g_far *far)
{
    struct gtp5g_outer_header_creation *hdr_creation = (far->fwd_param ? far->fwd_param->hdr_creation : NULL);
    return (hdr_creation ? &hdr_creation->port : NULL);
}
EXPORT_SYMBOL(gtp5g_far_get_outer_header_creation_port);

char *gtp5g_far_get_fwd_policy(struct gtp5g_far *far)
{
    struct gtp5g_forwarding_policy *fwd_policy = (far->fwd_param ? far->fwd_param->fwd_policy : NULL);
    return (fwd_policy ? fwd_policy->identifier : NULL);
}
EXPORT_SYMBOL(gtp5g_far_get_fwd_policy);

int *gtp5g_far_get_related_pdr_num(struct gtp5g_far *far)
{
    return &far->related_pdr_num;
}
EXPORT_SYMBOL(gtp5g_far_get_related_pdr_num);

uint16_t *gtp5g_far_get_related_pdr_list(struct gtp5g_far *far)
{
    return far->related_pdr_list;
}
EXPORT_SYMBOL(gtp5g_far_get_related_pdr_list);


/* QER */

/** 8.2.75 QER ID   
 *	Octets5~8
 *		-> QER ID Value
 *			 => BIT8 of Octet5 indicate Dynamically allocated by the CP(BIT8=0) or
 *				predefined in the UP(BIT8=1)
 * */
void gtp5g_qer_set_id(struct gtp5g_qer *qer, uint32_t id)
{
    qer->id = id;
}
EXPORT_SYMBOL(gtp5g_qer_set_id);

/* 8.2.7 Gate Status
 * 	Octet5 
 *		-Spare(4bits)
 *		-UL Gate(2bits)
 *		-DL Gate(2bits)
 * 	Value(Decimal):
 *		0 - OPEN (Default UL & DL)
 *		1 -	CLOSED
 *	
 * */
void gtp5g_qer_set_gate_status(struct gtp5g_qer *qer, uint8_t ul_dl_gate)
{
    qer->ul_dl_gate = (ul_dl_gate & 0x0F);
}
EXPORT_SYMBOL(gtp5g_qer_set_gate_status);

/* 8.2.8 MBR
 * 	Octet5~9, 	UL MBR
 * 	Octet10~14, DL MBR
 *
 * NOTE1: Encode as kilobits per second (1kbps = 1000bps) 
 * NOTE2: The range of UL/DL MBR is specified in 3GPP TS 36.413 [10].
 * NOTE3: The encoding is aligned on the encoding specified in 3GPP TS 29.274 [9]
 * */
void gtp5g_qer_set_mbr_uhigh(struct gtp5g_qer *qer, uint32_t high)
{
    qer->mbr.ul_high = high;
}
EXPORT_SYMBOL(gtp5g_qer_set_mbr_uhigh);

void gtp5g_qer_set_mbr_ulow(struct gtp5g_qer *qer, uint8_t low)
{
    qer->mbr.ul_low = low;
}
EXPORT_SYMBOL(gtp5g_qer_set_mbr_ulow);

void gtp5g_qer_set_mbr_dhigh(struct gtp5g_qer *qer, uint32_t high)
{
    qer->mbr.dl_high = high;
}
EXPORT_SYMBOL(gtp5g_qer_set_mbr_dhigh);

void gtp5g_qer_set_mbr_dlow(struct gtp5g_qer *qer, uint8_t low)
{
    qer->mbr.dl_low = low;
}
EXPORT_SYMBOL(gtp5g_qer_set_mbr_dlow);

/* 8.2.9 GBR
 *
 * */
void gtp5g_qer_set_gbr_uhigh(struct gtp5g_qer *qer, uint32_t high)
{
    qer->gbr.ul_high = high;
}
EXPORT_SYMBOL(gtp5g_qer_set_gbr_uhigh);

void gtp5g_qer_set_gbr_ulow(struct gtp5g_qer *qer, uint8_t low)
{
    qer->gbr.ul_low = low;
}
EXPORT_SYMBOL(gtp5g_qer_set_gbr_ulow);

void gtp5g_qer_set_gbr_dhigh(struct gtp5g_qer *qer, uint32_t high)
{
    qer->gbr.dl_high = high;
}
EXPORT_SYMBOL(gtp5g_qer_set_gbr_dhigh);

void gtp5g_qer_set_gbr_dlow(struct gtp5g_qer *qer, uint8_t low)
{
    qer->gbr.dl_low = low;
}
EXPORT_SYMBOL(gtp5g_qer_set_gbr_dlow);

/* 8.2.10 QER Correlation ID
 *
 * */
void gtp5g_qer_set_qer_corr_id(struct gtp5g_qer *qer, uint32_t qer_corr_id)
{
	qer->qer_corr_id = qer_corr_id;
}
EXPORT_SYMBOL(gtp5g_qer_set_qer_corr_id);

/* 8.2.88 RQI
 * 	Octet5:
 *		-> Spare(7bits)
 *		-> RQI (1bit)
 *			0 - Deactivate Reflective QoS (Default)
 *			1 - Activate Reflective QoS
 * */
void gtp5g_qer_set_rqi(struct gtp5g_qer *qer, uint8_t rqi)
{
	qer->rqi = rqi;
}
EXPORT_SYMBOL(gtp5g_qer_set_rqi);

/* 8.2.89 QFI
 *	-> It contains an QoS flow identifier identifying a QoS flow in a 5G 
 *		system filter
 * Octet5,
 *		-> Spare (2bits)
 *		-> QFI (6bits)
 * */
void gtp5g_qer_set_qfi(struct gtp5g_qer *qer, uint8_t qfi)
{
	qer->qfi = (qfi & 0x3F);
}
EXPORT_SYMBOL(gtp5g_qer_set_qfi);

/* 8.2.116 Paging Policy Indicator (PPI)
 *	-> Indicates a value for paging policy differentiation
 * Octet5,
 *		-> Spare (5bits)
 *		-> PPI (3bits)
 * */
void gtp5g_qer_set_ppi(struct gtp5g_qer *qer, uint8_t ppi)
{
	qer->ppi = (ppi & 0x07);
}
EXPORT_SYMBOL(gtp5g_qer_set_ppi);

/* 8.2.174 QER Control Indications
 *	-> Indicates a value for paging policy differentiation
 * Octet5,
 *		-> Spare (7bits)
 *		-> RCSR (1bit)
 *			1 -> UP function SHALL report the rate control status when the
 *				PFCP session released
 *			0 -> UP function SHALL NOT report the rate control status when the
 *				PFCP session released
 *			 
 * */
void gtp5g_qer_set_rcsr(struct gtp5g_qer *qer, uint8_t rcsr)
{
	qer->rcsr = (rcsr & 0x01);
}
EXPORT_SYMBOL(gtp5g_qer_set_rcsr);

void gtp5g_pdr_set_qer_id(struct gtp5g_pdr *pdr, uint32_t qer_id) {
    qer_id_may_alloc(pdr);
    *pdr->qer_id = qer_id;
}
EXPORT_SYMBOL(gtp5g_pdr_set_qer_id);

uint32_t *gtp5g_qer_get_id(struct gtp5g_qer *qer)
{
    return &qer->id;
}
EXPORT_SYMBOL(gtp5g_qer_get_id);

void gtp5g_qer_free(struct gtp5g_qer *qer)
{
	free(qer);
}
EXPORT_SYMBOL(gtp5g_qer_free);


