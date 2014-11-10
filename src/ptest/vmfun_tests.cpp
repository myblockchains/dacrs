// Copyright (c) 2011-2014 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include <stdlib.h>
#include <time.h>
#include "main.h"
#include "miner.h"
#include "uint256.h"
#include "util.h"
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>
#include "VmScript/VmScriptRun.h"
#include "VmScript/CVir8051.h"
#include "VmScript/TestMcu.h"
#include "json/json_spirit_writer_template.h"
#include "rpcclient.h"
using namespace std;
using namespace boost;
using namespace json_spirit;

extern Object CallRPC(const string& strMethod, const Array& params);
extern int TestCallRPC(std::string strMethod, const std::vector<std::string> &vParams, std::string &strRet);
extern void GetAccountInfo(char *address);
extern void GenerateMiner();
	//
//	string strPrint;
//	int nRet;
//	Array params = RPCConvertValues(strMethod, vParams);
//
//	Object reply = CallRPC(strMethod, params);
//
//	// Parse reply
//	const Value& result = find_value(reply, "result");
//	const Value& error = find_value(reply, "error");
//
//	if (error.type() != null_type) {
//		// Error
//		strPrint = "error: " + write_string(error, false);
//		int code = find_value(error.get_obj(), "code").get_int();
//		nRet = abs(code);
//	} else {
//		// Result
//		if (result.type() == null_type)
//			strPrint = "";
//		else if (result.type() == str_type)
//			strPrint = result.get_str();
//		else
//			strPrint = write_string(result, true);
//	}
//	strRet = strPrint;
//	BOOST_MESSAGE(strPrint);
//	//cout << strPrint << endl;
//	return nRet;
//}
//static void GetAccountInfo(char *address) {
//	int argc = 3;
//	char *argv[3] = { "rpctest", "getaccountinfo", address };
//	CommandLineRPC(argc, argv);
//
//}
//static void GenerateMiner() {
//	int argc = 3;
//	char *argv[3] = { "rpctest", "setgenerate", "true" };
//	CommandLineRPC(argc, argv);
//}


std::string TxHash("");
void GenerateMiner(int count) {
	//cout <<"Generate miner" << endl;
	int argc = 3;
	char buffer[10] = {0};
	sprintf(buffer,"%d",count);
	char *argv[4] = { "rpctest", "setgenerate", "true", buffer};
	CommandLineRPC(argc, argv);
}
void GetAccountState1() {
	GetAccountInfo("5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG");
}
void CreateRegScriptTx1() {
	cout <<"CreateRegScriptTx1" << endl;
	int argc = 6;
	char *argv[6] =
			{ "rpctest", "registerscripttx", "5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG",
					"fd6208020053000000000022220000000000000000222202081ee4900f2f784179018002f0a3d8fcd9fa7a007b0f900062783079018015e493a3ad82ae838a828b83f0a3aa82ab838d828e83d8e9d9e712069b12085e75d0007581bf750c00750d0f020017357a51506343315970464d7477786948373837705358616e5545436f4773785571334b5a69654a7856470001000000e0faa3e0fba3e0fca3e0fd22cac0e0e6f0a308dafad0e0fa22cac0e0e0a3c582ccc582c583cdc583f0a3c582ccc582c583cdc583dae6d0e0ca22250c10af08f50c400c150d8008f50c4002150dd2af2200c0d0250c10af08f50c500c050d8008f50c5002050dd2afd0d022250cf58210af08f50c400c150d8008f50c4002150dd2af850d8322c582250cc582c583350d10af07f50d85820c8007f50d85820cd2afc58322250cf582e4350df58322250cfae4350dfb22250cfce4350dfd22c8250cc8c9350dc922ca250ccacb350dcb22cc250ccccd350dcd22250cc582c0e0e50d34ffc583c0e0e50cc3958224f910af0885830d85820c800885830d85820cd2afcef0a3e520f0a37808e608f0a3defaeff0a3e58124fbf8e608f0a3e608f0a30808e608f0a3e608f0a315811581d0e0fed0e0f815811581e8c0e0eec0e022850d83850c82e0a3fee0a3f5207808e0a3f608dffae0a3ffe0a3c0e0e0a3c0e0e0a3c0e0e0a3c0e010af0885820c85830d800885820c85830dd2afd083d0822274028000c0e0f4041200fcd0e012009d2274028000ccc0e0edc0e0e50cc39cccad0d50011d10af068c0c8d0d80068c0c8d0dd2af1200aad0e0fdd0e0fc2274f812016ae990fbfef01200087f010201d0c082c083ea90fbfef0eba3f0120012020359ea2cf8eb3df9c3e89ae99b4007c3e89ce99d5004d2f08002c2f0a2f02274f512016a74fc1200cb850c82850d83ecf0a3edf08a088b0990f7fee0fea3e0ff850c82850d83e02ef8a3e03ff9e82402f85001097402120135e8f0a3e9f0850c82850d83e02402fca3e03400fdeefaeffb12027240127c027d00850c82850d8312035e1202725005790012024e750a02750b00780a12021074041201477afe7bf71207b774021200e174fe2efe74f73fff780a1202107402120147ee2402fae43ffb1207b774021200e1850c82850d83120221ac08ad09ee2404fae43ffb1207b774021200e1020364d083d08222e0faa3e0fb2274041200e17f040201d0740102036674f512016a74ff1200cbe91203b31202107c007d007afe7bf71207f474021200e17c017d00aa0cab0d12028feefceffdaa08ab0912028f7a097b0012026080bb850c82850d83f08a088b09ecfeedff750a00750b02780a2274f512016aeafeebff8c088d09750a00750b02780a1202107c007d007afe7bf71207f474021200e17c207d00eefaeffb12028f7c027d00740b12013f12028f7a0a7b00120260740b120135c082c083120550d083d0821205494004e8497003c3801790f7fe1202217c007df8aa08ab091207b774021200e1d302036974f512016aeafeebff8c088d09750a00750b02780a1202107c007d007afe7bf71207f474021200e17c207d00eefaeffb12028f7c027d00740b12013f12028f7a0b7b00120260740b120135c082c083120550d083d0821205494004e8497003c3801790f7fe1202217c007df8aa08ab091207b774021200e1d302036974f512016aeafeebff8c088d09750a00750b02780a1202107c007d007afe7bf71207f474021200e1eefaeffb1208421205be12028f7a0c7b00120260740b120135c082c083120550d083d082120549401890f7fee064417002a3e0600f90f7fee064217002a3e06003c3801790f7fe1202217c007df8aa08ab091207b774021200e1d3020369c3e098a3e0992290f7fee0f8a3e0f92274f512016aeafeebff8c088d09750a00750b02780a1202107c007d007afe7bf71207f474021200e1eefaeffb1208421205be12028f7a0d7b0012026090f7fee064087002a3e06003c3801790f7fe1202217c007df8aa08ab091207b774021200e1d30203698b0bea2401fce4350bfdeefaeffb2274041200e17f020201d074f712016a74fc1200cb1206421202107c007d007afe7bf71207f474021200e17c047d00aa0cab0d12028f7a107b00120260740f120135c082c083120550d083d0821205494004e8497003c3801b90f7fe1202217c007df8740f12013512035e1207b774021200e1d3808b850c82850d83eaf0a3ebf0a3ecf0a3edf075080075090278082274f712016a8a088b09ecfeedff7a197b00120260120550c3ee98ef994004e8497003c3801790f7fe1202217c007df8aa08ab091207b774021200e1d30205d27582b87583fd120117900f2f7c287d0212016174201200aa900f4f740812014774201200aa7c20fd740812013f12065c79017c207d00740812013f1203737508207509007808120210782a790212014f880889097808120210900f2b1200911205d774041200e179017c207d007a287b021201581203737508007509027808120210742a120147740a12013f1203cb74021200e179017c007d02742812013f1203737808120210742a120147740a12013f12044774021200e179017c007d02742812013f1203737508417509007808120210742a1201477a007b0f1204c374021200e179017c007d02742812013f120373ac0cad0d7a007b0f12055979017c087d00aa0cab0d120373790112024e7a007b007582487583021201172274f812016aeaf8ebf97408120135e0fea3e0801f8c828d83e088828983f0a3a882a9838c828d83a3ac82ad83ee24ff1eef34ffffee4f70dc7f010201d0c082c083850c82850d83e0f8a3e0f9e84960128a828b83ecf0a3e824ff18e934fff94870f2d083d0822274f812016a74fe1200cbeafeebff850c82850d83eef0a3eff0aa0cab0d790112086180eac082c0838a828b838001a3e070fce582c39afae5839bfbd083d082220200142200",
					"1000000", "2" };
	CommandLineRPC(argc, argv);
}
bool CreateTx()
{
	int argc = 8;
	std::vector<std::string> vInputParams;
	vInputParams.clear();
	vInputParams.push_back("010000000100");
	vInputParams.push_back(
			"[\"5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d\"]");
	vInputParams.push_back("0b434b430046003531303030303030300000000000000000000000");
	vInputParams.push_back("100000000");
	vInputParams.push_back("10");
	std::string strReturn("");
	if (TestCallRPC("createcontracttx", vInputParams, strReturn) > 0) {
		vInputParams.clear();
		cout << "create secure tx succeed1:"<<strReturn<< endl;
		TxHash = strReturn;
	}
	return false;
}
void ListRegScript1() {
	//cout << "listRegScript" << endl;
	int argc = 2;
	char *argv[2] = { "rpctest", "listregscript" };
	CommandLineRPC(argc, argv);
}
void CreateRegScriptTx2() {
	cout <<"CreateRegScriptTx1" << endl;
	int argc = 6;
	char *argv[6] =
			{ "rpctest", "registerscripttx", "5zQPcC1YpFMtwxiH787pSXanUECoGsxUq3KZieJxVG",
					"fd031602005300000000002222000000000000000022220215dbe4901166780b79018002f0a3d8fcd9fa7a007b0f900062786779038015e493a3ad82ae838a828b83f0a3aa82ab838d828e83d8e9d9e7120e3c1215ff75d0007581bf750e00750f0f020017ffffffffffffffff0000000000010074657374616464206572726f7200000000000200fffffffffffffffe000100746573746d756c206572726f7200ffffffffffffff0100040030786666fffffffffffc0400ffffff0100307833660ffffc040074657374737562206572726f72000200ffffffffffffffc10074657374646976206572726f72006b65790068656c6c6f005772697465446174614442206572726f72006b657931007368656c6c6f00526561644461746156616c75654442206572726f7200526561644461746156616c756544422076616c75653a005265616444617461444254696d65206572726f72005265616444617461444254696d652076616c7565206572726f7200735265616444617461444254696d65206572726f7200476574444253697a652076616c7565206572726f7200476574444256616c7565206572726f7200476574444256616c7565206b65793a00476574444256616c75652076616c75653a00476574444256616c75652074696d653a006c756f004d6f64696679446174614442206572726f723a004d6f646966794461746144422072656164206572726f723a006a696e004d6f646966794461746144425661766c65206572726f723a00526561644461746156616c75654442206572726f723a005265616444617461444254696d65206572726f723a0044656c657465446174614442206572726f723a0064656c657465206166746572205265616444617461444254696d65206572726f723a00746573747365636f6e64646220476574444253697a652076616c7565206572726f72000200000005000000000000000100000003000000e066701008a3e066700a08a3e066700408a3e06622c3e09608a3e09608a3e09608a3e09622e0f608a3e0f608a3e0f608a3e0f622e0faa3e0fba3e0fca3e0fd22cac0e0e6f0a308dafad0e0fa22cac0e0e0a3c582ccc582c583cdc583f0a3c582ccc582c583cdc583dae6d0e0ca22250e10af08f50e400c150f8008f50e4002150fd2af2200c0d0250e10af08f50e500c050f8008f50e5002050fd2afd0d022250ef58210af08f50e400c150f8008f50e4002150fd2af850f8322250ef582e4350ff58322250ef8e4350ff922250efae4350ffb22250efce4350ffd22250ec582c0e0e50f34ffc583c0e0e50ec3958224f910af0885830f85820e800885830f85820ed2afcef0a3e520f0a37808e608f0a3defaeff0a3e58124fbf8e608f0a3e608f0a30808e608f0a3e608f0a315811581d0e0fed0e0f815811581e8c0e0eec0e022850f83850e82e0a3fee0a3f5207808e0a3f608dffae0a3ffe0a3c0e0e0a3c0e0e0a3c0e0e0a3c0e010af0885820e85830f800885820e85830fd2afd083d0822274028000c0e0f404120367d0e0120308227404800474028000ccc0e0edc0e0e50ec39cccad0f50011d10af068c0e8d0f80068c0e8d0fd2af120315d0e0fdd0e0fc2274f71203a474d812033679087c007d0faa0eab0f120f9e79057c097d0f7418120394120f9e79057c667d117420120394120f9e740812038c88088909780812044a741a12039c74021203941210cf740212034c742012039c7408120394120fffe9600d79007c0e7d007a0f7b0f1210f479087c097d0faa0eab0f120f9e79057c097d0f7418120394120f9e79057c1d7d0f7410120394120f9e780812044a740212039c74021203941210cf740212034c741012039c7408120394120fffe9600d79007c0e7d007a0f7b0f1210f479087c237d0faa0eab0f120f9e79057c097d0f7418120394120f9e79057c007d0f7410120394120f9e780812044a740212039c74021203941210cf740212034c741012039c7408120394120fffe9600d79007c0e7d007a0f7b0f1210f402085174f71203a474d812033679087c007d0faa0eab0f120f9e79087c007d0f7418120394120f9e79017c2c7d0f7420120394120f9e740812038c88088909780812044a741a12039c74021203941210bd740212034c742012039c7408120394120fffe9600d79007c0e7d007a2e7b0f1210f479087c3c7d0faa0eab0f120f9e79017c457d0f7418120394120f9e79057c477d0f7410120394120f9e780812044a740212039c74021203941210bd740212034c741012039c7408120394120fffe9600d79007c0e7d007a2e7b0f1210f479047c537d0faa0eab0f120f9e79017c457d0f7418120394120f9e79057c587d0f7410120394120f9e780812044a740212039c74021203941210bd740212034c741012039c7408120394120fffe9600d79007c0e7d007a2e7b0f1210f402085174f71203a474e012033679087c007d0f7418120394120f9e79087c007d0f7410120394120f9e79017c6c7d11aa0eab0f120f9e740812038c88088909780812044a741212039c741a1203941210bd740212034cac0ead0f7408120394120fffe9600d79007c0e7d007a617b0f1210f479017c2c7d0f7410120394120f9e79087c237d0faa0eab0f120f9e780812044a741212039c741a1203941210bd740212034cac0ead0f7408120394120fffe9600d79007c0e7d007a617b0f1210f479017c2c7d0f7410120394120f9e79017c6f7d0faa0eab0f120f9e780812044a741a12039c74121203941210bd740212034cac0ead0f7408120394120fffe9600d79007c0e7d007a617b0f1210f479017c2c7d0f7418120394120f9e79087c6f7d0f7410120394120f9e79087c007d0faa0eab0f120f9e780812044a741212039c741a1203941210bd740212034cac0ead0f7408120394120fffe9600d79007c0e7d007a617b0f1210f474208002742812034c7f0202040a74f71203a474e012033679087c007d0f7418120394120f9e79017c2c7d0f7408120394120f9e79017c717d0f7410120394120f9ea80ea90f88088909780812044a740a12039c741a1203941210bd740212034c741012039caa0eab0f120fffe9600d79007c0e7d007a7a7b0f1210f479017c6c7d117410120394120f9e780812044a741a12039c740a1203941210bd740212034c741012039caa0eab0f120fffe9600d79007c0e7d007a7a7b0f1210f479017c6c7d117408120394120f9e780812044a741a12039c740a1203941210bd740212034c500d79007c0e7d007a7a7b0f1210f402084d74f51203a474e212033690115212045b750806750900780812044a7c8c7d0f79047a887b0f121157740612034c400d79007c127d007a927b0f1210f490115212045b750807780812044a7ca97d0f79057aa47b0f121157740612034c400d79007c127d007a927b0f1210f4750806780812044a741312039c79047a887b0f1211be740212034c8b09ea4509700c79007c167d007ab07b0f801d7c8c7d0f74111203941215358b09ea4509700d79007c177d007ac67b0f1210f4ac0ead0f79047a887b0f12141c400c79007c157d007add7b0f801f90115278081202ed850e82850f8378081202c8600d79007c1b7d007af27b0f1210f4750807750900780812044a741912039c79057aa47b0f1211be740212034c8b09ea4509700c79007c167d007ab07b0f801d7ca97d0f74171203941215358b09ea4509700d79007c177d007ac67b0f1210f4ac0ead0f79057aa47b0f12141c400c79007c167d007a0d7b10801f90115678081202ed850e82850f8378081202c8600d79007c1b7d007af27b0f1210f49011521212658a088b098c0a8d0b78081202c8600d79007c167d007a237b101210f4740412038c88088909780812044a740212038c88088909780812044a750806750900780812044a741712038c88088909780812044a741312038c88088909780812044a790090115a1202fc1212c3740a12034c7405120382e0f8a3e0f9e84960087404120382e0700c79007c117d007a397b10805f7ca47d0f740b1203941215358b09ea4509700d79007c107d007a4a7b101210f47ca97d0f740b1203941215358b09ea4509700d79007c107d007a5a7b101210f490115278081202ed850e82850f8378081202c8600d79007c117d007a6c7b101210f4740412038c88088909780812044a740212038c88088909780812044a750807750900780812044a741d12038c88088909780812044a741312038c88088909780812044a790090115a1202fc1212c3740a12034c7405120382e0f8a3e0f9e84960087404120382e0700c79007c117d007a397b10805f7ca47d0f740b1203941215358b09ea4509700d79007c107d007a4a7b101210f47ca97d0f740b1203941215358b09ea4509700d79007c107d007a5a7b101210f490115678081202ed850e82850f8378081202c8600d79007c117d007a6c7b101210f4850e82850f837401120e1c90115e12045b750804750900780812044a7c7d7d1079047a887b0f12121c740612034c400c79007c147d007a817b108057780812044a740912039c79047a887b0f1211be740212034cac0ead0f79047a887b0f12141c7c7d7d1074071203941215358b09ea4509702290115e78081202ed850e82850f8378081202c8600d79007c197d007a957b101210f4750804750900780812044a7cae7d1079047a887b0f1214b0740212034c400c79007c197d007ab27b108035780812044a740912039c79047a887b0f1211be740212034c7cae7d1074071203941215358b09ea4509700d79007c177d007acb7b101210f4850e82850f837403120e1c90116212045b79047a887b0f12146d740412034c5022ac0ead0f79047a887b0f12141c90116278081202ed850e82850f8378081202c8600d79007c167d007ae27b101210f479047a887b0f121196400c79007c147d007af87b108019ac0ead0f79047a887b0f12141c500d79007c237d007a0c7b111210f4741e12034c7f0402040af0a3e4f0a3f0a3f022121265ea4b4c4d600d79007c237d007a2f7b111210f42274fe12033690116eac0ead0f740212031590115212045b74041203941214e3740412034c850e82850f83e06401701a12048c1205b91206e612085b1209427901120e93740212034c22e064027005120e2580eb790080e974f81203a4e990effdf012000802103d74f81203a4e990effdf0a3e4f012001202103dea2cf8eb3df9c3e89ae99b4007c3e89ce99d5004d2f08002c2f0a2f02274f51203a474fc120336850e82850f83ecf0a3edf08a088b0990efffe0fea3e0ff850e82850f83e02ef8a3e03ff9e82402f85001097402120382e8f0a3e9f0850e82850f83e02402fca3e03400fdeefaeffb120eb6400c7c027d0012113d120eb650077900120e938059750a02750b00780a12044a740412039c7aff7bef121574740212034c74ff2e1e74ef3fff780a12044a740212039cee2402fae43ffb121574740212034c850e82850f8312045fac08ad09ee2404fae43ffb121574740212034c740412034c02115274f31203a48a088b09ecfeedff890a750c08750d00780c12044a7c007d001215b1740212034ce50ac394094027c37f0602040a8e828f83e0c0e0850a0ce508250cf582e43509f583d0e0f08e828f83a3ae82af83a80a74ff28f50a0470d5d380cd74f81203a4ecfeedff90efffe4f0a3f07c08fd120ed37c087d00eefaeffb120ed37900120ea390efffe064017002a3e0700790f001e0f980057900120e937f0102040a74f51203a4ecfeedff890890efffe4f0a3f07c08fd120ed37c087d00eefaeffb120ed3a908120ea390efffc3e09409a3e09400503e740b120382e0fea3e0ff750a08750b00780a12044a7c007d00eefaeffb1215b1740212034c90efff12045f7c017df0eefaeffb121574740212034cd380057900120e93021152c082c083850e82850f8312045f79010210e7c082c083850e82850f8312045f79020210e7e0fca3e0fd22121042740212034cd083d0822274f71203a474ff120336e9850e82850f83f08a088b09ecfeedff90efffe4f0a3f07c01fdaa0eab0f120ed3eefceffdaa08ab09120ed37909120ea374018002740412034c7f0202040a850e82850f83e0faa3e0fb2290efffe0f8a3e0f9227f0402040a74f61203a412145c120ed3740a12038212125b120ed37c047d00740c120394120ed37913120ea390efffe064017002a3e06003c3800690f001e0a2e002145774f71203a41214a1120ed37914120ea390efffe064017002a3e06003c3800690f001e0a2e002113874f51203a4ecfeedff90efffe4f0a3f0890aac0afd120ed37915120ea3740b120382c082c083121149d083d08212121550067a007b00801c90efff12045f7c017df0eefaeffb121574740212034c90efff121143021152c3e098a3e0992274f61203a412145c120ed3740a12038212125b120ed37c047d00740c120394120ed37916120ea390efffe064017002a3e06003c3800690f001e0a2e0021457e0fca3e0fdeefaeffb2274f71203a474fc1203367917120ea390efffe064047002a3e0600a7a007b007c007d0080201212ad12044a7c017df07402120394121574740212034c850e82850f831202fc021133850e82850f83e4f0a3f0a3f0a3f0750804f50978082274f71203a474f91203367403120382121405750803f509780812044a7c007d0074021203941215b1740212034c7c047d007403120394120ed37918120ea390efffc3e09409a3e0940040030213b21213fa70030213b290efff12045f7c017df07412120382121143121574740212034c90efffe0850e82850f83f090efffe02401fea3e034f0ff8e828f83e0f508a3e0f5097414120382c3e09508a3e09509404e780812044aee2402fce43ffd7414120382121143121574740212034c8e828f8312114c7401120382e8f0a3e9f08e828f8312114cee28f8ef39f9e82402fee439ff8e828f83e064047002a3e06024850e82850f83c082c08374181203821210e1d083d082ecfaedfb74031203157407021135750804750900780812044aee2402fce43ffd7418120382121143121574740212034c80b890efffe0f8a3e0f9e84922eaf0a3ebf0a3ecf0a3ed12141322f090efffe4f0a3f02274f61203a412145c120ed3791a120ea390efffe064047002a3e06003c3801c750804750900780812044a7c017df0eefaeffb121574740212034cd37f0302040aecfeedff90efffe4f0a3f08908ac08fd2274f71203a41214a1120ed37c047d007409120394120ed3791b120ea390efffe064017002a3e06003c3800690f001e0a2e0021138e9fe90efffe4f0a3f08e08ac08fd2274f61203a412145c120ed3740a12038212125b120ed3791c120ea390efffe064017002a3e06003c3800690f001e0a2e002145774f51203a41213fa601b90efffe0f508a3e0f509e4f50af50b740b12038278081202dd500a7a007b007c007d00802090efff12045f7c017df0121574740212034c90efffe0f508a3e0aa08fbe4fcfd021152c082c083800ea3aa82ab838c828d83a3ac82ad838c828d83e0f88a828b83e0f9e8697007e970df7a00800de0c39850067aff7bff80047a017b00d083d0822274f81203a4eaf8ebf97408120382e0fea3e0801f8c828d83e088828983f0a3a882a9838c828d83a3ac82ad83ee24ff1eef34ffffee4f70dc7f0102040ac082c083850e82850f83e0f8a3e0f9e84960128a828b83ecf0a3e824ff18e934fff94870f2d083d0822274f81203a474fe120336eafeebff850e82850f83eef0a3eff0aa0eab0f790112160280ea0200142200",
					"1000000", "2" };
	CommandLineRPC(argc, argv);
}
void CreateFirstTx()
{
	int argc = 8;
	std::vector<std::string> vInputParams;
	vInputParams.clear();
	vInputParams.push_back("010000000100");
	vInputParams.push_back(
			"[\"5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d\"]");
	vInputParams.push_back("01");
	vInputParams.push_back("100000000");
	vInputParams.push_back("10");
	std::string strReturn("");
	if (TestCallRPC("createcontracttx", vInputParams, strReturn) > 0) {
		vInputParams.clear();
		cout << "create secure tx succeed1:"<<strReturn<< endl;
		TxHash = strReturn;
	}
	return ;
}
void CreateSecondTx()
{
	int argc = 8;
	std::vector<std::string> vInputParams;
	vInputParams.clear();
	vInputParams.push_back("010000000100");
	vInputParams.push_back(
			"[\"5yNhSL7746VV5qWHHDNLkSQ1RYeiheryk9uzQG6C5d\"]");
	vInputParams.push_back("02");
	vInputParams.push_back("100000000");
	vInputParams.push_back("10");
	std::string strReturn("");
	if (TestCallRPC("createcontracttx", vInputParams, strReturn) > 0) {
		vInputParams.clear();
		cout << "create secure tx succeed1:"<<strReturn<< endl;
		TxHash = strReturn;
	}
	return ;
}
BOOST_AUTO_TEST_SUITE(VM_fun)

BOOST_AUTO_TEST_CASE(Gloal_fun)
{
	//cout << "=====================init account info ========================" << endl;
	GetAccountState1();
	CreateRegScriptTx1();
	GenerateMiner();
	cout << "=====================create tx 1========================" << endl;
	ListRegScript1();
	CreateTx();
	GenerateMiner();
//	GetAccountState1();
}

BOOST_AUTO_TEST_CASE(test_fun)
{
	GetAccountState1();
	CreateRegScriptTx2();
	GenerateMiner();
	cout << "=====================create tx 1========================" << endl;
	ListRegScript1();
	CreateFirstTx();
	GenerateMiner(10);

	CreateSecondTx();
//	GetAccountState1();
}
BOOST_AUTO_TEST_SUITE_END()
