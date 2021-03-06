#ifndef TS_H
#define TS_H

#include "buffer.h"
#include "protocol.h"
#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <string>

class TsFile
{
public:
	enum
	{
		TS_PACKET_SIZE = 188
	};
	struct AdaptionField
	{
		unsigned adaption_field_length         : 8;
		unsigned discontinuity_indicator       : 1;
		unsigned random_access_indicator       : 1;
		unsigned elementary_stream_priority_indicator : 1;
		unsigned PCR_flag                      : 1;
		unsigned OPCR_flag                     : 1;
		unsigned splicing_point_flag           : 1;
		unsigned transport_private_data_flag   : 1;
		unsigned adaption_field_extension_flag : 1;

		//optional
		uint64_t PCR;
		uint64_t OPCR;
		unsigned splice_countdown              : 8;
		unsigned transport_private_data_length : 8;
		std::vector<unsigned char> transport_private_data;
		//adaption_extension

		AdaptionField()
			:adaption_field_length(0),
			discontinuity_indicator(0),
			random_access_indicator(0),
			elementary_stream_priority_indicator(0),
			PCR_flag(0),
			OPCR_flag(0),
			splicing_point_flag(0),
			transport_private_data_flag(0),
			adaption_field_extension_flag(0),
			PCR(0),
			OPCR(0),
			splice_countdown(0),
			transport_private_data_length(0)
		{
		}
		void Analyze(BitBuffer &bits);
	};
	struct PacketHeader
	{
		unsigned sync_byte:8;

		unsigned transport_error_indicator:1;
		unsigned payload_unit_start_indicator:1;
		unsigned transport_priority:1;
		unsigned pid:13;
		unsigned transport_scrambling_control:2;
		 //1 no
		 //2,adaption
		 //3,adaption + payload
		unsigned adaptation_field_control:2;
		unsigned continuity_counter:4;

		AdaptionField *adaption;

		PacketHeader()
			:sync_byte(0),
			transport_error_indicator(0),
			payload_unit_start_indicator(0),
			transport_priority(0),
			pid(0),
			transport_scrambling_control(0),
			adaptation_field_control(0),
			continuity_counter(0),
			adaption(NULL)
		{

		}
		~PacketHeader()
		{
			if (adaption != NULL)
			{
				delete adaption;
				adaption = NULL;
			}
		}
		const char *GetPidName() const;
		void Analyze(BitBuffer &bits);
		void Dump();
	};
	struct SDTService
	{
		unsigned service_id                   : 16;
		unsigned EIT_schedule_flag            : 1;
		unsigned EIT_present_following_flag   : 1;
		unsigned running_status               : 3;
		unsigned freed_CA_mode                : 1;
		unsigned descriptors_loop_length      : 12;
		//std::vector<unsigned char> descriptor;

		//service descriptor expansion
		unsigned descriptor_tag               : 8;
		unsigned descriptor_length            : 8;
		unsigned service_type                 : 8;
		unsigned service_provider_name_length : 8;
		std::string service_provider_name;
		unsigned service_name_length : 8;
		std::string service_name;
	};
	struct SDT
	{
		unsigned table_id                     : 8;
		unsigned section_syntax_indicator     : 1;
		unsigned reserved_1                   : 1;
		unsigned reserved_2                   : 2;
		unsigned section_length               : 12;
		unsigned transport_stream_id          : 16;
		unsigned reserved_3                   : 2;
		unsigned version_number               : 5;
		unsigned current_next_indicator       : 1;
		unsigned section_number               : 8;
		unsigned last_section_number          : 8;
		unsigned original_network_id          : 16;
		unsigned reserved_4                   : 8;
		std::vector<SDTService> vecService;
		unsigned CRC_32                       : 32; //CRC32校验码

		SDT()
			:table_id(0),
			section_syntax_indicator(0),
			reserved_1(0),
			reserved_2(0),
			section_length(0),
			transport_stream_id(0),
			reserved_3(0),
			version_number(0),
			current_next_indicator(0),
			section_number(0),
			last_section_number(0),
			reserved_4(0),
			CRC_32(0)
		{
		}
		void Analyze(BitBuffer &bits, unsigned payload);
		void Dump();
	};
	struct PATProgram
	{
		unsigned program_number:  16;  //节目号
		unsigned program_map_PID: 13;  //节目映射表的PID，节目号大于0时对应的PID，每个节目对应一个
	};
	struct PAT
	{
		unsigned table_id                     : 8;  //固定为0x00 ，标志是该表是PAT表
		unsigned section_syntax_indicator     : 1;  //段语法标志位，固定为1
		unsigned zero                         : 1;  //0
		unsigned reserved_1                   : 2;  // 保留位
		unsigned section_length               : 12; //表示从下一个字段开始到CRC32(含)之间有用的字节数
		unsigned transport_stream_id          : 16; //该传输流的ID，区别于一个网络中其它多路复用的流
		unsigned reserved_2                   : 2;  // 保留位
		unsigned version_number               : 5;  //范围0-31，表示PAT的版本号
		unsigned current_next_indicator       : 1;  //发送的PAT是当前有效还是下一个PAT有效
		unsigned section_number               : 8;  //分段的号码。PAT可能分为多段传输，第一段为00，以后每个分段加1，最多可能有256个分段
		unsigned last_section_number          : 8;  //最后一个分段的号码

		std::vector<PATProgram> vecProgram;
		unsigned reserved_3                   : 3;  // 保留位
		unsigned network_PID                  : 13; //网络信息表（NIT）的PID,节目号为0时对应的PID为network_PID
		unsigned CRC_32                       : 32; //CRC32校验码

		PAT()
			:table_id(0),
			section_syntax_indicator(0),
			zero(0),
			reserved_1(0),
			section_length(0),
			transport_stream_id(0),
			reserved_2(0),
			version_number(0),
			current_next_indicator(0),
			section_number(0),
			last_section_number(0),
			reserved_3(0),
			network_PID(0),
			CRC_32(0)
		{

		}
		void Analyze(BitBuffer &bits, unsigned payload);
		void Dump();
	};
	struct PMTStream
	{
		unsigned stream_type                  : 8; //指示特定PID的节目元素包的类型。该处PID由elementary PID指定
		unsigned elementary_PID               : 13; //该域指示TS包的PID值。这些TS包含有相关的节目元素
		unsigned ES_info_length               : 12; //前两位bit为00。该域指示跟随其后的描述相关节目元素的byte数
		unsigned descriptor;
	};
	struct PMT
	{
		unsigned table_id                     : 8; //固定为0x02, 表示PMT表
		unsigned section_syntax_indicator     : 1; //固定为0x01
		unsigned zero                         : 1; //0x01
		unsigned reserved_1                   : 2; //0x03
		unsigned section_length               : 12;//表示从下一个字段开始到CRC32(含)之间有用的字节数
		unsigned program_number               : 16;//指出该节目对应于可应用的Program map PID
		unsigned reserved_2                   : 2; //0x03
		unsigned version_number               : 5; //指出TS流中Program map section的版本号
		unsigned current_next_indicator       : 1; //当该位置1时，当前传送的Program map section可用；
												   //当该位置0时，指示当前传送的Program map section不可用，下一个TS流的Program map section有效。
		unsigned section_number               : 8; //固定为0x00
		unsigned last_section_number          : 8; //固定为0x00
		unsigned reserved_3                   : 3; //0x07
		unsigned PCR_PID                      : 13; //指明TS包的PID值，该TS包含有PCR域，
		//该PCR值对应于由节目号指定的对应节目。
		//如果对于私有数据流的节目定义与PCR无关，这个域的值将为0x1FFF。
		unsigned reserved_4                   : 4; //预留为0x0F
		unsigned program_info_length          : 12; //前两位bit为00。该域指出跟随其后对节目信息的描述的byte数。

		std::vector<PMTStream> vecStream;//每个元素包含8位, 指示特定PID的节目元素包的类型。该处PID由elementary PID指定
		//unsigned reserved_5                   : 3; //0x07
		//unsigned reserved_6                   : 4; //0x0F
		unsigned CRC_32                       : 32;

		PMT()
		:table_id(0),
		section_syntax_indicator(0),
		zero(0),
		reserved_1(0),
		section_length(0),
		program_number(0),
		reserved_2(0),
		version_number(0),
		current_next_indicator(0),
		section_number(0),
		last_section_number(0),
		reserved_3(0),
		PCR_PID(0),
		reserved_4(0),
		program_info_length(0),
		//reserved_5(0),
		//reserved_6(0),
		CRC_32(0)
		{

		}
		void Analyze(BitBuffer &bits, unsigned payload);
		void Dump();
		const char *GetTypeName(unsigned type) const;
		unsigned GetVideoPid() const;
		unsigned GetAudioPid() const;
		unsigned GetVideoStreamType() const;
		unsigned GetAudioStreamType() const;
		bool IsVideoStreamType(unsigned stream_type) const;
		bool IsAudioStreamType(unsigned stream_type) const;
	};
	explicit TsFile(IProtocol *protocol);
	~TsFile();
	bool ReadPacket();
	bool AnalyzePacket();
	bool IsPMT(unsigned pid) const;
	int GetPacketNum() const
	{
		return mPacket;
	}
private:
	IProtocol *mProtocol;
	unsigned mPacket;
	unsigned char mBuffer[TS_PACKET_SIZE];
	BitBuffer mBits;

	SDT mSDT;
	PAT mPAT;
	PMT mPMT; //sometimes maybe more than one

	unsigned mVideoPid;
	unsigned mAudioPid;
	unsigned mVideoStreamType;
	unsigned mAudioStreamType;
	std::vector<unsigned char> mVideoBuffer;
	std::vector<unsigned char> mAudioBuffer;
};

#endif
