/*
 * Copyright (C) 2019,2020 by Sukchan Lee <acetcom@gmail.com>
 *
 * This file is part of Open5GS.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "npcf-build.h"

ogs_sbi_request_t *smf_npcf_smpolicycontrol_build_create(
        smf_sess_t *sess, void *data)
{
    smf_ue_t *smf_ue = NULL;

    ogs_sbi_message_t message;
    ogs_sbi_header_t header;
    ogs_sbi_request_t *request = NULL;
    ogs_sbi_server_t *server = NULL;

    OpenAPI_sm_policy_context_data_t SmPolicyContextData;
    OpenAPI_ambr_t SubsSessAmbr;
    OpenAPI_subscribed_default_qos_t SubsDefQos;
    OpenAPI_arp_t Arp;
    OpenAPI_snssai_t sNssai;

    ogs_assert(sess);
    ogs_assert(sess->sm_context_ref);
    smf_ue = sess->smf_ue;
    ogs_assert(smf_ue);

    memset(&message, 0, sizeof(message));
    message.h.method = (char *)OGS_SBI_HTTP_METHOD_POST;
    message.h.service.name = (char *)OGS_SBI_SERVICE_NAME_NPCF_SMPOLICYCONTROL;
    message.h.api.version = (char *)OGS_SBI_API_V1;
    message.h.resource.component[0] = (char *)OGS_SBI_RESOURCE_NAME_SM_POLICIES;

    memset(&SmPolicyContextData, 0, sizeof(SmPolicyContextData));
    memset(&sNssai, 0, sizeof(sNssai));
    memset(&SubsSessAmbr, 0, sizeof(SubsSessAmbr));

    SmPolicyContextData.supi = smf_ue->supi;
    if (!SmPolicyContextData.supi) {
        ogs_error("No supi");
        goto end;
    }
    SmPolicyContextData.pdu_session_id = sess->psi;
    if (!SmPolicyContextData.pdu_session_id) {
        ogs_error("No pdu_session_id");
        goto end;
    }
    SmPolicyContextData.pdu_session_type = sess->session.session_type;
    if (!SmPolicyContextData.pdu_session_type) {
        ogs_error("No pdu_session_type");
        goto end;
    }
    SmPolicyContextData.dnn = sess->session.name;
    if (!SmPolicyContextData.dnn) {
        ogs_error("No dnn");
        goto end;
    }

    server = ogs_list_first(&ogs_sbi_self()->server_list);
    if (!server) {
        ogs_error("No server");
        goto end;
    }

    memset(&header, 0, sizeof(header));
    header.service.name = (char *)OGS_SBI_SERVICE_NAME_NSMF_CALLBACK;
    header.api.version = (char *)OGS_SBI_API_V1;
    header.resource.component[0] =
            (char *)OGS_SBI_RESOURCE_NAME_SM_POLICY_NOTIFY;
    header.resource.component[1] = sess->sm_context_ref;
    SmPolicyContextData.notification_uri = ogs_sbi_server_uri(server, &header);
    if (!SmPolicyContextData.notification_uri) {
        ogs_error("No notification_uri");
        goto end;
    }

    if (sess->ipv4) {
        SmPolicyContextData.ipv4_address =
                ogs_ipv4_to_string(sess->ipv4->addr[0]);
        if (!SmPolicyContextData.ipv4_address) {
            ogs_error("No ipv4_address");
            goto end;
        }
    }

    if (sess->ipv6) {
        SmPolicyContextData.ipv6_address_prefix = ogs_ipv6prefix_to_string(
                (uint8_t *)sess->ipv6->addr, OGS_IPV6_128_PREFIX_LEN);
        if (!SmPolicyContextData.ipv6_address_prefix) {
            ogs_error("No ipv6_address_prefix");
            goto end;
        }
    }

    if (sess->session.ipv4_framed_routes) {
        int i;
        OpenAPI_list_t *FrameRouteList = OpenAPI_list_create();

        for (i = 0; i < OGS_MAX_NUM_OF_FRAMED_ROUTES_IN_PDI; i++) {
            const char *route = sess->session.ipv4_framed_routes[i];
            if (!route) break;
            OpenAPI_list_add(FrameRouteList, ogs_strdup(route));
        }
        SmPolicyContextData.ipv4_frame_route_list = FrameRouteList;
    }

    if (sess->session.ipv6_framed_routes) {
        int i;
        OpenAPI_list_t *FrameRouteList = OpenAPI_list_create();

        for (i = 0; i < OGS_MAX_NUM_OF_FRAMED_ROUTES_IN_PDI; i++) {
            const char *route = sess->session.ipv6_framed_routes[i];
            if (!route) break;
            OpenAPI_list_add(FrameRouteList, ogs_strdup(route));
        }
        SmPolicyContextData.ipv6_frame_route_list = FrameRouteList;
    }

    if (OGS_SBI_FEATURES_IS_SET(sess->smpolicycontrol_features,
                OGS_SBI_NPCF_SMPOLICYCONTROL_DN_AUTHORIZATION)) {
        if (sess->session.ambr.uplink) {
            SubsSessAmbr.uplink = ogs_sbi_bitrate_to_string(
                sess->session.ambr.uplink, OGS_SBI_BITRATE_KBPS);
        }
        if (sess->session.ambr.downlink) {
            SubsSessAmbr.downlink = ogs_sbi_bitrate_to_string(
                sess->session.ambr.downlink, OGS_SBI_BITRATE_KBPS);
        }
        if (SubsSessAmbr.downlink || SubsSessAmbr.uplink) {
            SmPolicyContextData.subs_sess_ambr = &SubsSessAmbr;
        }
    }

    memset(&Arp, 0, sizeof(Arp));
    if (sess->session.qos.arp.pre_emption_capability ==
            OGS_5GC_PRE_EMPTION_ENABLED)
        Arp.preempt_cap = OpenAPI_preemption_capability_MAY_PREEMPT;
    else if (sess->session.qos.arp.pre_emption_capability ==
            OGS_5GC_PRE_EMPTION_DISABLED)
        Arp.preempt_cap = OpenAPI_preemption_capability_NOT_PREEMPT;
    if (!Arp.preempt_cap) {
        ogs_error("No Arp.preempt_cap");
        goto end;
    }
    if (sess->session.qos.arp.pre_emption_vulnerability ==
            OGS_5GC_PRE_EMPTION_ENABLED)
        Arp.preempt_vuln = OpenAPI_preemption_vulnerability_PREEMPTABLE;
    else if (sess->session.qos.arp.pre_emption_vulnerability ==
            OGS_5GC_PRE_EMPTION_DISABLED)
        Arp.preempt_vuln = OpenAPI_preemption_vulnerability_NOT_PREEMPTABLE;
    if (!Arp.preempt_vuln) {
        ogs_error("No Arp.preempt_vuln");
        goto end;
    }
    Arp.priority_level = sess->session.qos.arp.priority_level;

    memset(&SubsDefQos, 0, sizeof(SubsDefQos));
    SubsDefQos.arp = &Arp;
    SubsDefQos._5qi = sess->session.qos.index;
    SubsDefQos.is_priority_level = true;
    SubsDefQos.priority_level = sess->session.qos.arp.priority_level;

    SmPolicyContextData.subs_def_qos = &SubsDefQos;

    if (sess->smpolicycontrol_features) {
        SmPolicyContextData.supp_feat =
            ogs_uint64_to_string(sess->smpolicycontrol_features);
        if (!SmPolicyContextData.supp_feat) {
            ogs_error("No supp_feat");
            goto end;
        }
    }

    sNssai.sst = sess->s_nssai.sst;
    sNssai.sd = ogs_s_nssai_sd_to_string(sess->s_nssai.sd);
    SmPolicyContextData.slice_info = &sNssai;

    message.SmPolicyContextData = &SmPolicyContextData;

    request = ogs_sbi_build_request(&message);
    ogs_expect(request);

end:
    if (SmPolicyContextData.notification_uri)
        ogs_free(SmPolicyContextData.notification_uri);
    if (SmPolicyContextData.gpsi)
        ogs_free(SmPolicyContextData.gpsi);

    if (sNssai.sd)
        ogs_free(sNssai.sd);

    if (SubsSessAmbr.downlink) ogs_free(SubsSessAmbr.downlink);
    if (SubsSessAmbr.uplink) ogs_free(SubsSessAmbr.uplink);

    if (SmPolicyContextData.supp_feat)
        ogs_free(SmPolicyContextData.supp_feat);

    if (SmPolicyContextData.ipv4_address)
        ogs_free(SmPolicyContextData.ipv4_address);
    if (SmPolicyContextData.ipv6_address_prefix)
        ogs_free(SmPolicyContextData.ipv6_address_prefix);

    OpenAPI_clear_and_free_string_list(SmPolicyContextData.ipv4_frame_route_list);
    OpenAPI_clear_and_free_string_list(SmPolicyContextData.ipv6_frame_route_list);

    return request;
}

ogs_sbi_request_t *smf_npcf_smpolicycontrol_build_delete(
        smf_sess_t *sess, void *data)
{
    smf_npcf_smpolicycontrol_param_t *param = data;

    smf_ue_t *smf_ue = NULL;

    ogs_sbi_message_t message;
    ogs_sbi_request_t *request = NULL;

    OpenAPI_sm_policy_delete_data_t SmPolicyDeleteData;
    OpenAPI_list_t *ranNasRelCauseList = NULL;
    OpenAPI_ran_nas_rel_cause_t *ranNasRelCause = NULL;
    OpenAPI_user_location_t ueLocation;
    OpenAPI_lnode_t *node = NULL;

    ogs_assert(sess);
    ogs_assert(sess->sm_context_ref);
    smf_ue = sess->smf_ue;
    ogs_assert(smf_ue);
    ogs_assert(sess->policy_association_id);

    memset(&message, 0, sizeof(message));
    message.h.method = (char *)OGS_SBI_HTTP_METHOD_POST;
    message.h.service.name = (char *)OGS_SBI_SERVICE_NAME_NPCF_SMPOLICYCONTROL;
    message.h.api.version = (char *)OGS_SBI_API_V1;
    message.h.resource.component[0] = (char *)OGS_SBI_RESOURCE_NAME_SM_POLICIES;
    message.h.resource.component[1] = sess->policy_association_id;
    message.h.resource.component[2] = (char *)OGS_SBI_RESOURCE_NAME_DELETE;

    memset(&SmPolicyDeleteData, 0, sizeof(SmPolicyDeleteData));

    memset(&ueLocation, 0, sizeof(ueLocation));

    if (param) {
        if (param->ran_nas_release.gmm_cause ||
            param->ran_nas_release.gsm_cause ||
            param->ran_nas_release.ngap_cause.group) {

            ranNasRelCauseList = OpenAPI_list_create();
            if (!ranNasRelCauseList) {
                ogs_error("No ranNasRelCauseList");
                goto end;
            }

            ranNasRelCause = ogs_calloc(1, sizeof(*ranNasRelCause));
            if (!ranNasRelCause) {
                ogs_error("No ranNasRelCause");
                goto end;
            }

            if (param->ran_nas_release.ngap_cause.group) {
                OpenAPI_ng_ap_cause_t *ngApCause = NULL;

                ranNasRelCause->ng_ap_cause = ngApCause =
                    ogs_calloc(1, sizeof(*ngApCause));
                if (!ranNasRelCause->ng_ap_cause) {
                    ogs_error("No ranNasRelCause->ng_ap_cause");
                    if (ranNasRelCause)
                        ogs_free(ranNasRelCause);
                    goto end;
                }

                ngApCause->group = param->ran_nas_release.ngap_cause.group;
                ngApCause->value = param->ran_nas_release.ngap_cause.value;
            }

            ranNasRelCause->is__5g_mm_cause = true;
            ranNasRelCause->_5g_mm_cause = param->ran_nas_release.gmm_cause;
            ranNasRelCause->is__5g_sm_cause = true;
            ranNasRelCause->_5g_sm_cause = param->ran_nas_release.gsm_cause;

            OpenAPI_list_add(ranNasRelCauseList, ranNasRelCause);
        }

        // if (param->ue_location) {
        //     ueLocation.nr_location = ogs_sbi_build_nr_location(
        //             &sess->nr_tai, &sess->nr_cgi);
        //     if (!ueLocation.nr_location) {
        //         ogs_error("ueLocation.nr_location");
        //         goto end;
        //     }
        //     ueLocation.nr_location->ue_location_timestamp =
        //         ogs_sbi_gmtime_string(sess->ue_location_timestamp);
        //     if (!ueLocation.nr_location->ue_location_timestamp) {
        //         ogs_error("ueLocation.nr_location->ue_location_timestamp");
        //         goto end;
        //     }

        //     SmPolicyDeleteData.user_location_info = &ueLocation;
        // }
        if (param->ue_timezone) {
            SmPolicyDeleteData.ue_time_zone =
                ogs_sbi_timezone_string(ogs_timezone());
            if (!SmPolicyDeleteData.ue_time_zone) {
                ogs_error("SmPolicyDeleteData.ue_time_zone");
                goto end;
            }
        }
    }

    SmPolicyDeleteData.serving_network =
        ogs_sbi_build_plmn_id_nid(&sess->plmn_id);
    if (!SmPolicyDeleteData.serving_network) {
        ogs_error("SmPolicyDeleteData.serving_network");
        goto end;
    }

    SmPolicyDeleteData.ran_nas_rel_causes = ranNasRelCauseList;

    message.SmPolicyDeleteData = &SmPolicyDeleteData;

    request = ogs_sbi_build_request(&message);
    ogs_expect(request);

end:

    if (ueLocation.nr_location) {
        if (ueLocation.nr_location->ue_location_timestamp)
            ogs_free(ueLocation.nr_location->ue_location_timestamp);
        ogs_sbi_free_nr_location(ueLocation.nr_location);
    }
    if (SmPolicyDeleteData.ue_time_zone)
        ogs_free(SmPolicyDeleteData.ue_time_zone);

    OpenAPI_list_for_each(ranNasRelCauseList, node) {
        ranNasRelCause = node->data;
        if (ranNasRelCause) {
            if (ranNasRelCause->ng_ap_cause) {
                ogs_free(ranNasRelCause->ng_ap_cause);
            }
            ogs_free(ranNasRelCause);
        }
    }

    OpenAPI_list_free(ranNasRelCauseList);

    if (SmPolicyDeleteData.serving_network)
        ogs_sbi_free_plmn_id_nid(SmPolicyDeleteData.serving_network);

    return request;
}
