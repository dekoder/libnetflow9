#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <netflow9.h>
#include <netinet/in.h>
#include <tins/tins.h>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include "test_lib.h"

class pcap_test : public test
{
protected:
    std::vector<parse_result> parse_pcap(std::string path)
    {
        auto packets = get_packets(path.c_str());
        std::vector<parse_result> parsed;

        for (const auto &packet : packets) {
            nf9_parse_result *result;
            if (nf9_parse(state_, &result, packet.data_.data(),
                          packet.data_.size(), &packet.addr))
                continue;
            parsed.emplace_back(result);
        }

        return parsed;
    }
};

TEST_F(pcap_test, basic_test)
{
    std::vector<parse_result> parsed_pcap = parse_pcap("testcases/1.pcap");
    nf9_addr addr = nf9_get_addr(parsed_pcap.at(0).get());
    EXPECT_EQ(addr.in.sin_addr.s_addr, inet_addr("172.17.0.5"));
    std::vector<uint32_t> src_ips;
    for (const auto &pr : parsed_pcap) {
        for (size_t flowset = 0; flowset < nf9_get_num_flowsets(pr.get());
             ++flowset) {
            for (size_t flow = 0; flow < nf9_get_num_flows(pr.get(), flowset);
                 ++flow) {
                nf9_value field = nf9_get_field(pr.get(), flowset, flow,
                                                NF9_FIELD_IPV4_SRC_ADDR);
                src_ips.push_back(field.u32);
            }
        }
    }

    EXPECT_EQ(src_ips.size(), 0);  // 2
}

TEST_F(pcap_test, basic_stats_test)
{
    std::vector<parse_result> pr = parse_pcap("testcases/1.pcap");
    stats st = get_stats();

    EXPECT_EQ(nf9_get_stat(st.get(), NF9_STAT_TOTAL_RECORDS), 4);
    EXPECT_EQ(nf9_get_stat(st.get(), NF9_STAT_TOTAL_TEMPLATES), 2);
    EXPECT_EQ(nf9_get_stat(st.get(), NF9_STAT_TOTAL_OPTION_TEMPLATES), 2);
    EXPECT_EQ(nf9_get_stat(st.get(), NF9_STAT_MISSING_TEMPLATE_ERRORS), 0);
    EXPECT_EQ(nf9_get_stat(st.get(), NF9_STAT_MALFORMED_PACKETS), 0);
}

TEST_F(pcap_test, malformed_1_test)
{
    std::vector<parse_result> pr = parse_pcap("testcases/malformed_1.pcap");
    stats st = get_stats();

    EXPECT_EQ(nf9_get_stat(st.get(), NF9_STAT_MALFORMED_PACKETS), 0 /* 3 */);
}

TEST_F(pcap_test, malformed_2_test)
{
    std::vector<parse_result> pr = parse_pcap("testcases/malformed_2.pcap");
    stats st = get_stats();

    EXPECT_EQ(nf9_get_stat(st.get(), NF9_STAT_MALFORMED_PACKETS), 0 /* 19 */);
}

TEST_F(pcap_test, malformed_3_test)
{
    std::vector<parse_result> pr = parse_pcap("testcases/malformed_3.pcap");
    stats st = get_stats();

    EXPECT_EQ(nf9_get_stat(st.get(), NF9_STAT_MALFORMED_PACKETS), 0 /* 20 */);
}

TEST_F(pcap_test, malformed_4_test)
{
    /* The PCAP contains a Netflow packet where one flowset
     * has length that equals zero.
     */
    std::vector<parse_result> pr = parse_pcap("testcases/malformed_4.pcap");
    stats st = get_stats();

    EXPECT_EQ(nf9_get_stat(st.get(), NF9_STAT_MALFORMED_PACKETS), 1);
}

TEST_F(pcap_test, malformed_5_test)
{
    /* The PCAP contains a Netflow packet where one flowset has no
     * option fields and scope field with length equals zero.
     */
    std::vector<parse_result> pr = parse_pcap("testcases/malformed_5.pcap");
    stats st = get_stats();

    EXPECT_EQ(nf9_get_stat(st.get(), NF9_STAT_MALFORMED_PACKETS), 0 /* 1 */);
}

TEST_F(pcap_test, template_matching_test)
{
    // Netflow::UniqueStreamID test_id_1 = {256, 104, "2.1.3.8"};
    // Netflow::UniqueStreamID test_id_2 = {257, 104, "172.17.0.5"};
    // Netflow::UniqueStreamID test_id_3 = {258, 104, "172.17.0.5"};

    // EXPECT_EQ(processor.template_library().count(), 4);
    // EXPECT_EQ(processor.template_library().exists(test_id_1), false);
    // EXPECT_EQ(processor.template_library().exists(test_id_2), true);
    // EXPECT_EQ(processor.template_library().exists(test_id_3), true);
}