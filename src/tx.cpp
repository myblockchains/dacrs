#include "serialize.h"
#include <boost/foreach.hpp>
#include "hash.h"
#include "util.h"
#include "database.h"
#include "main.h"
#include <algorithm>
#include "txdb.h"
#include "vm/vmrunevn.h"
#include "core.h"
#include "miner.h"
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_writer_template.h"
using namespace json_spirit;

string txTypeArray[] = { "NULL_TXTYPE", "REWARD_TX", "REG_ACCT_TX", "COMMON_TX", "CONTRACT_TX", "REG_APP_TX"};


bool CID::Set(const CRegID &id) {
	CDataStream ds(SER_DISK, CLIENT_VERSION);
	ds << id;
	vchData.clear();
	vchData.insert(vchData.end(), ds.begin(), ds.end());
	return true;
}
bool CID::Set(const CKeyID &id) {
	vchData.resize(20);
	memcpy(&vchData[0], &id, 20);
	return true;
}
bool CID::Set(const CPubKey &id) {
	vchData.resize(id.size());
	memcpy(&vchData[0], &id, id.size());
	return true;
}
bool CID::Set(const CNullID &id) {
	return true;
}
bool CID::Set(const CUserID &userid) {
	return boost::apply_visitor(CIDVisitor(this), userid);
}
CUserID CID::GetUserId() {
	if (1< vchData.size() && vchData.size() <= 10) {
		CRegID regId;
		regId.SetRegIDByCompact(vchData);
		return CUserID(regId);
	} else if (vchData.size() == 33) {
		CPubKey pubKey(vchData);
		return CUserID(pubKey);
	} else if (vchData.size() == 20) {
		uint160 data = uint160(vchData);
		CKeyID keyId(data);
		return CUserID(keyId);
	} else if(vchData.empty()) {
		return CNullID();
	}
	else {
		LogPrint("ERROR", "vchData:%s, len:%d\n", HexStr(vchData).c_str(), vchData.size());
		assert(0);
	}
	return CNullID();
}


bool CRegID::clean()  {
	nHeight = 0 ;
	nIndex = 0 ;
	vRegID.clear();
	return true;
}
CRegID::CRegID(const vector<unsigned char>& vIn) {
	assert(vIn.size() == 6);
	vRegID = vIn;
	nHeight = 0;
	nIndex = 0;
	CDataStream ds(vIn, SER_DISK, CLIENT_VERSION);
	ds >> nHeight;
	ds >> nIndex;
}
bool CRegID::IsSimpleRegIdStr(const string & str)
{
	int len = str.length();
	if (len >= 3) {
		int pos = str.find('-');

		if (pos > len - 1) {
			return false;
		}
		string firtstr = str.substr(0, pos);

		if (firtstr.length() > 10 || firtstr.length() == 0) //int max is 4294967295 can not over 10
			return false;

		for (auto te : firtstr) {
			if (!isdigit(te))
				return false;
		}
		string endstr = str.substr(pos + 1);
		if (endstr.length() > 10 || endstr.length() == 0) //int max is 4294967295 can not over 10
			return false;
		for (auto te : endstr) {
			if (!isdigit(te))
				return false;
		}
	}
	return true;
}
bool CRegID::GetKeyID(const string & str,CKeyID &keyId)
{
	CRegID te(str);
	if(te.IsEmpty())
		return false;
	keyId = te.getKeyID(*pAccountViewTip);
	return !keyId.IsEmpty();
}
bool CRegID::IsRegIdStr(const string & str)
 {
	if(IsSimpleRegIdStr(str)){
		return true;
	}
	else if(str.length()==12){
		return true;
	}
	return false;
}
void CRegID::SetRegID(string strRegID){
	nHeight = 0;
	nIndex = 0;
	vRegID.clear();

	if(IsSimpleRegIdStr(strRegID))
	{
		int pos = strRegID.find('-');
		nHeight = atoi(strRegID.substr(0, pos).c_str());
		nIndex = atoi(strRegID.substr(pos+1).c_str());
		vRegID.insert(vRegID.end(), BEGIN(nHeight), END(nHeight));
		vRegID.insert(vRegID.end(), BEGIN(nIndex), END(nIndex));
//		memcpy(&vRegID.at(0),&nHeight,sizeof(nHeight));
//		memcpy(&vRegID[sizeof(nHeight)],&nIndex,sizeof(nIndex));
	}
	else if(strRegID.length()==12)
	{
	vRegID = ::ParseHex(strRegID);
	memcpy(&nHeight,&vRegID[0],sizeof(nHeight));
	memcpy(&nIndex,&vRegID[sizeof(nHeight)],sizeof(nIndex));
	}

}
void CRegID::SetRegID(const vector<unsigned char>& vIn) {
	assert(vIn.size() == 6);
	vRegID = vIn;
	CDataStream ds(vIn, SER_DISK, CLIENT_VERSION);
	ds >> nHeight;
	ds >> nIndex;
}
CRegID::CRegID(string strRegID) {
	SetRegID(strRegID);
}
CRegID::CRegID(uint32_t nHeightIn, uint16_t nIndexIn) {
	nHeight = nHeightIn;
	nIndex = nIndexIn;
	vRegID.clear();
	vRegID.insert(vRegID.end(), BEGIN(nHeightIn), END(nHeightIn));
	vRegID.insert(vRegID.end(), BEGIN(nIndexIn), END(nIndexIn));
}
string CRegID::ToString() const {
//	if(!IsEmpty())
//	return ::HexStr(vRegID);
	if(!IsEmpty())
	  return  strprintf("%d-%d",nHeight,nIndex);
	return string(" ");
}
CKeyID CRegID::getKeyID(const CAccountViewCache &view)const
{
	CKeyID ret;
	CAccountViewCache(view).GetKeyId(*this,ret);
	return ret;
}
void CRegID::SetRegIDByCompact(const vector<unsigned char> &vIn) {
	if(vIn.size()>0)
	{
		CDataStream ds(vIn, SER_DISK, CLIENT_VERSION);
		ds >> *this;
	}
	else
	{
		clean();
	}
}


bool CBaseTransaction::IsValidHeight(int nCurHeight, int nTxCacheHeight) const
{
	if(REWARD_TX == nTxType)
		return true;
	if (nValidHeight > nCurHeight + nTxCacheHeight / 2)
			return false;
	if (nValidHeight < nCurHeight - nTxCacheHeight / 2)
			return false;
	return true;
}
bool CBaseTransaction::UndoExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {
	vector<CAccountLog>::reverse_iterator rIterAccountLog = txundo.vAccountLog.rbegin();
	for (; rIterAccountLog != txundo.vAccountLog.rend(); ++rIterAccountLog) {
		CAccount account;
		CUserID userId = rIterAccountLog->keyID;
		if (!view.GetAccount(userId, account)) {
			return state.DoS(100, ERRORMSG("UndoExecuteTx() : undo ExecuteTx read accountId= %s account info error"),
					UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
		}
		if (!account.UndoOperateAccount(*rIterAccountLog)) {
			return state.DoS(100, ERRORMSG("UndoExecuteTx() : undo UndoOperateAccount failed"), UPDATE_ACCOUNT_FAIL,
					"undo-operate-account-failed");
		}
		if (COMMON_TX == nTxType
				&& (account.IsEmptyValue()
						&& (!account.PublicKey.IsFullyValid() || account.PublicKey.GetKeyID() != account.keyID))) {
			view.EraseAccount(userId);
		} else {
			if (!view.SetAccount(userId, account)) {
				return state.DoS(100,
						ERRORMSG("UndoExecuteTx() : undo ExecuteTx write accountId= %s account info error"),
						UPDATE_ACCOUNT_FAIL, "bad-write-accountdb");
			}
		}
	}
	vector<CScriptDBOperLog>::reverse_iterator rIterScriptDBLog = txundo.vScriptOperLog.rbegin();
	for (; rIterScriptDBLog != txundo.vScriptOperLog.rend(); ++rIterScriptDBLog) {
		if (!scriptCache.UndoScriptData(rIterScriptDBLog->vKey, rIterScriptDBLog->vValue))
			return state.DoS(100, ERRORMSG("UndoExecuteTx() : undo scriptdb data error"), UPDATE_ACCOUNT_FAIL,
					"bad-save-scriptdb");
	}
	if(CONTRACT_TX == nTxType) {
		if (!scriptCache.EraseTxRelAccout(GetHash()))
			return state.DoS(100, ERRORMSG("UndoExecuteTx() : erase tx rel account error"), UPDATE_ACCOUNT_FAIL,
							"bad-save-scriptdb");
	}
	return true;
}
uint64_t CBaseTransaction::GetFuel(int nfuelRate) {
	uint64_t llFuel = ceil(nRunStep/100.0f) * nfuelRate;
	if(REG_APP_TX == nTxType) {
		if (llFuel < 1 * COIN) {
			llFuel = 1 * COIN;
		}
	}
	return llFuel;
}

int CBaseTransaction::GetFuelRate() {
	if(0 == nFuelRate) {
		CDiskTxPos postx;
		if (pblocktree->ReadTxIndex(GetHash(), postx)) {
			CAutoFile file(OpenBlockFile(postx, true), SER_DISK, CLIENT_VERSION);
			CBlockHeader header;
			try {
				file >> header;
			} catch (std::exception &e) {
				return ERRORMSG("%s : Deserialize or I/O error - %s", __func__, e.what());
			}
			nFuelRate = header.nFuelRate;
		}
		else {
			nFuelRate = GetElementForBurn(chainActive.Tip());
		}
	}
	return nFuelRate;
}

bool CRegisterAccountTx::ExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {
	CAccount account;
	CRegID regId(nHeight, nIndex);
	CKeyID keyId = boost::get<CPubKey>(userId).GetKeyID();
	if (!view.GetAccount(userId, account))
		return state.DoS(100, ERRORMSG("ExecuteTx() : read source keyId %s account info error", keyId.ToString()),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	CAccountLog acctLog(account);
	if(account.PublicKey.IsFullyValid() && account.PublicKey.GetKeyID() == keyId) {
		return state.DoS(100, ERRORMSG("ExecuteTx() : read source keyId %s duplicate register", keyId.ToString()),
					UPDATE_ACCOUNT_FAIL, "duplicate-register-account");
	}
	account.PublicKey = boost::get<CPubKey>(userId);
	if (llFees > 0) {
		if(!account.OperateAccount(MINUS_FREE, llFees))
			return state.DoS(100, ERRORMSG("ExecuteTx() : not sufficient funds in account, keyid=%s", keyId.ToString()),
					UPDATE_ACCOUNT_FAIL, "not-sufficiect-funds");
	}

	account.regID = regId;
	if (typeid(CPubKey) == minerId.type()) {
		account.MinerPKey = boost::get<CPubKey>(minerId);
		if (account.MinerPKey.IsValid() && !account.MinerPKey.IsFullyValid()) {
			return state.DoS(100, ERRORMSG("ExecuteTx() : MinerPKey:%s Is Invalid", account.MinerPKey.ToString()),
					UPDATE_ACCOUNT_FAIL, "MinerPKey Is Invalid");
		}
	}

	if (!view.SaveAccountInfo(regId, keyId, account)) {
		return state.DoS(100, ERRORMSG("ExecuteTx() : write source addr %s account info error", regId.ToString()),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}
	txundo.vAccountLog.push_back(acctLog);
	txundo.txHash = GetHash();
	return true;
}
bool CRegisterAccountTx::UndoExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state,
		CTxUndo &txundo, int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {
	//drop account
	CRegID accountId(nHeight, nIndex);
	CAccount oldAccount;
	if (!view.GetAccount(accountId, oldAccount))
		return state.DoS(100,
				ERRORMSG("ExecuteTx() : read secure account=%s info error", accountId.ToString()),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	CKeyID keyId;
	view.GetKeyId(accountId, keyId);

	if (llFees > 0) {
		CAccountLog accountLog;
		if (!txundo.GetAccountOperLog(keyId, accountLog))
			return state.DoS(100, ERRORMSG("ExecuteTx() : read keyId=%s tx undo info error", keyId.GetHex()),
					UPDATE_ACCOUNT_FAIL, "bad-read-txundoinfo");
		oldAccount.UndoOperateAccount(accountLog);
	}

	if (!oldAccount.IsEmptyValue()) {
		CPubKey empPubKey;
		oldAccount.PublicKey = empPubKey;
		oldAccount.MinerPKey = empPubKey;
		CUserID userId(keyId);
		view.SetAccount(userId, oldAccount);
	} else {
		view.EraseAccount(userId);
	}
	view.EraseId(accountId);
	return true;
}
bool CRegisterAccountTx::GetAddress(set<CKeyID> &vAddr, CAccountViewCache &view) {
	if (!boost::get<CPubKey>(userId).IsFullyValid()) {
		return false;
	}
	vAddr.insert(boost::get<CPubKey>(userId).GetKeyID());
	return true;
}
string CRegisterAccountTx::ToString(CAccountViewCache &view) const {
	string str;
	str += strprintf("txType=%s, hash=%s, ver=%d, pubkey=%s, llFees=%ld, keyid=%s, nValidHeight=%d\n",
	txTypeArray[nTxType],GetHash().ToString().c_str(), nVersion, boost::get<CPubKey>(userId).ToString(), llFees, boost::get<CPubKey>(userId).GetKeyID().ToAddress(), nValidHeight);
	return str;
}
bool CRegisterAccountTx::CheckTransction(CValidationState &state, CAccountViewCache &view) {
	//check pubKey valid
	if (!boost::get<CPubKey>(userId).IsFullyValid()) {
		return state.DoS(100, ERRORMSG("CheckTransaction() : register tx public key is invalid"), REJECT_INVALID,
				"bad-regtx-publickey");
	}

	//check signature script
	uint256 sighash = SignatureHash();
	if(!CheckSignScript(sighash, signature, boost::get<CPubKey>(userId))) {
		return state.DoS(100, ERRORMSG("CheckTransaction() : register tx signature error "), REJECT_INVALID,
				"bad-regtx-signature");
	}

	if (!MoneyRange(llFees))
		return state.DoS(100, ERRORMSG("CheckTransaction() : register tx fee out of range"), REJECT_INVALID,
				"bad-regtx-fee-toolarge");
	return true;
}

bool CTransaction::ExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {
	CAccount srcAcct;
	CAccount desAcct;
	CAccountLog desAcctLog;
	uint64_t minusValue = llFees+llValues;
	if (!view.GetAccount(srcRegId, srcAcct))
		return state.DoS(100,
				ERRORMSG("ExecuteTx() : read source addr %s account info error", boost::get<CRegID>(srcRegId).ToString()),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	CAccountLog srcAcctLog(srcAcct);
	if (!srcAcct.OperateAccount(MINUS_FREE, minusValue))
		return state.DoS(100, ERRORMSG("ExecuteTx() : accounts insufficient funds"), UPDATE_ACCOUNT_FAIL,
				"bad-read-accountdb");
	CUserID userId = srcAcct.keyID;
	if(!view.SetAccount(userId, srcAcct)){
		return state.DoS(100, ERRORMSG("UpdataAccounts() :save account%s info error",  boost::get<CRegID>(srcRegId).ToString()),
				UPDATE_ACCOUNT_FAIL, "bad-write-accountdb");
	}

	uint64_t addValue = llValues;
	if(!view.GetAccount(desUserId, desAcct)) {
		if(COMMON_TX == nTxType) {
			desAcct.keyID = boost::get<CKeyID>(desUserId);
			desAcctLog.keyID = desAcct.keyID;
		}
		else {
			return state.DoS(100, ERRORMSG("ExecuteTx() : get account info failed by regid"), UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
		}
	}
	else{
		desAcctLog.SetValue(desAcct);
	}
	if (!desAcct.OperateAccount(ADD_FREE, addValue)) {
		return state.DoS(100, ERRORMSG("ExecuteTx() : operate accounts error"), UPDATE_ACCOUNT_FAIL,
				"bad-operate-account");
	}
	if (!view.SetAccount(desUserId, desAcct)) {
		return state.DoS(100, ERRORMSG("ExecuteTx() : save account error, kyeId=%s", desAcct.keyID.ToString()),
				UPDATE_ACCOUNT_FAIL, "bad-save-account");
	}
	txundo.vAccountLog.push_back(srcAcctLog);
	txundo.vAccountLog.push_back(desAcctLog);

	if (CONTRACT_TX == nTxType) {
		vector<unsigned char> vScript;
		if(!scriptCache.GetScript(boost::get<CRegID>(desUserId), vScript)) {
			return state.DoS(100, ERRORMSG("ExecuteTx() : save account error, kyeId=%s", desAcct.keyID.ToString()),
						UPDATE_ACCOUNT_FAIL, "bad-save-account");
		}
		CVmRunEvn vmRunEvn;
		std::shared_ptr<CBaseTransaction> pTx = GetNewInstance();
		uint64_t el = GetFuelRate();
		int64_t llTime = GetTimeMillis();
		tuple<bool, uint64_t, string> ret = vmRunEvn.run(pTx, view, scriptCache, nHeight, el, nRunStep);
		if (!std::get<0>(ret))
			return state.DoS(100,
					ERRORMSG("ExecuteTx() : ContractTransaction ExecuteTx txhash=%s run script error:%s",
							GetHash().GetHex(), std::get<2>(ret)), UPDATE_ACCOUNT_FAIL, "run-script-error");
		LogPrint("CONTRACT_TX", "execute contract elapse:%lld, txhash=%s\n", GetTimeMillis() - llTime,
				GetHash().GetHex());
		set<CKeyID> vAddress;
		vector<std::shared_ptr<CAccount> > &vAccount = vmRunEvn.GetNewAccont();
		for (auto & itemAccount : vAccount) {
			vAddress.insert(itemAccount->keyID);
			userId = itemAccount->keyID;
			CAccount oldAcct;
			if(!view.GetAccount(userId, oldAcct)) {
				return state.DoS(100,
							ERRORMSG("ExecuteTx() : ContractTransaction ExecuteTx read account info error"),
							UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
			}
			CAccountLog oldAcctLog(oldAcct);
			if (!view.SetAccount(userId, *itemAccount))
				return state.DoS(100,
						ERRORMSG("ExecuteTx() : ContractTransaction ExecuteTx write account info error"),
						UPDATE_ACCOUNT_FAIL, "bad-write-accountdb");
			txundo.vAccountLog.push_back(oldAcctLog);
		}
		txundo.vScriptOperLog.insert(txundo.vScriptOperLog.end(), vmRunEvn.GetDbLog()->begin(), vmRunEvn.GetDbLog()->end());
		if(!scriptCache.SetTxRelAccout(GetHash(), vAddress))
				return ERRORMSG("ExecuteTx() : ContractTransaction ExecuteTx save tx relate account info to script db error");

	}
	txundo.txHash = GetHash();
	return true;
}
bool CTransaction::GetAddress(set<CKeyID> &vAddr, CAccountViewCache &view) {
	CKeyID keyId;
	if(!view.GetKeyId(srcRegId, keyId))
		return false;
	vAddr.insert(keyId);
	CKeyID desKeyId;
	if(desUserId.type() == typeid(CKeyID)) {
		desKeyId = boost::get<CKeyID>(desUserId);
	} else if(desUserId.type() == typeid(CRegID)){
		if (!view.GetKeyId(desUserId, desKeyId))
			return false;
	} else
		return false;

	if (CONTRACT_TX == nTxType) {
		CVmRunEvn vmRunEvn;
		std::shared_ptr<CBaseTransaction> pTx = GetNewInstance();
		uint64_t el = GetFuelRate();
		CScriptDBViewCache scriptDBView(*pScriptDBTip, true);
		if (uint256(0) == pTxCacheTip->IsContainTx(GetHash())) {
			CAccountViewCache accountView(view, true);
			tuple<bool, uint64_t, string> ret = vmRunEvn.run(pTx, accountView, scriptDBView, chainActive.Height() + 1, el,
					nRunStep);
			if (!std::get<0>(ret))
				return ERRORMSG("GetAddress()  : %s", std::get<2>(ret));

			vector<shared_ptr<CAccount> > vpAccount = vmRunEvn.GetNewAccont();

			for (auto & item : vpAccount) {
				vAddr.insert(item->keyID);
			}
		} else {
			set<CKeyID> vTxRelAccount;
			if (!scriptDBView.GetTxRelAccount(GetHash(), vTxRelAccount))
				return false;
			vAddr.insert(vTxRelAccount.begin(), vTxRelAccount.end());
		}
	}
	return true;
}
string CTransaction::ToString(CAccountViewCache &view) const {
	string str;
	string desId;
	if (desUserId.type() == typeid(CKeyID)) {
		desId = boost::get<CKeyID>(desUserId).ToString();
	} else if (desUserId.type() == typeid(CRegID)) {
		desId = boost::get<CRegID>(desUserId).ToString();
	}
	str += strprintf("txType=%s, hash=%s, ver=%d, srcId=%s desId=%s, llFees=%ld, vContract=%s\n",
	txTypeArray[nTxType], GetHash().ToString().c_str(), nVersion, boost::get<CRegID>(srcRegId).ToString(), desId.c_str(), llFees, HexStr(vContract).c_str());
	return str;
}
bool CTransaction::CheckTransction(CValidationState &state, CAccountViewCache &view) {
	if (!MoneyRange(llFees)) {
		return state.DoS(100, ERRORMSG("CheckTransaction() : appeal tx fee out of range"), REJECT_INVALID,
				"bad-appeal-fee-toolarge");
	}

	CAccount acctInfo;
	if (!view.GetAccount(boost::get<CRegID>(srcRegId), acctInfo)) {
		return state.DoS(100, ERRORMSG("CheckTransaction() :tx GetAccount falied"), REJECT_INVALID, "bad-getaccount");
	}
	if (!acctInfo.IsRegister()) {
		return state.DoS(100, ERRORMSG("CheckTransaction(): account have not registed public key"), REJECT_INVALID,
				"bad-no-pubkey");
	}

	//check signature script
	uint256 sighash = SignatureHash();
	if (!CheckSignScript(sighash, signature, acctInfo.PublicKey)) {
		return state.DoS(100, ERRORMSG("CheckTransaction() :CheckSignScript failed"), REJECT_INVALID,
				"bad-signscript-check");
	}

	return true;
}
uint256 CTransaction::SignatureHash() const  {
	CHashWriter ss(SER_GETHASH, 0);
	CID srcId(srcRegId);
	CID desId(desUserId);
	ss <<VARINT(nVersion) << nTxType << VARINT(nValidHeight);
	ss << srcId << desId << VARINT(llFees) << vContract;
	return ss.GetHash();
}


bool CRewardTransaction::ExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {
	CID id(account);
	if (account.type() != typeid(CRegID) && account.type() != typeid(CPubKey)) {
		return state.DoS(100,
				ERRORMSG("ExecuteTx() : account  %s error, either accountId, or pubkey", HexStr(id.GetID())),
				UPDATE_ACCOUNT_FAIL, "bad-account");
	}
	CAccount acctInfo;
	if (!view.GetAccount(account, acctInfo)) {
		return state.DoS(100, ERRORMSG("ExecuteTx() : read source addr %s account info error", HexStr(id.GetID())),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}
	CAccount acctInfoLog(acctInfo);
	if(0 == nIndex) {   //current block reward tx, need to clear coindays
		acctInfo.ClearAccPos(nHeight);
	}
	else if(-1 == nIndex){ //maturity reward tx,only update values
		acctInfo.llValues += rewardValue;
	}
	else {  //never go into this step
		assert(0);
	}

	CUserID userId = acctInfo.keyID;
	if (!view.SetAccount(userId, acctInfo))
		return state.DoS(100, ERRORMSG("ExecuteTx() : write secure account info error"), UPDATE_ACCOUNT_FAIL,
				"bad-save-accountdb");
	txundo.Clear();
	txundo.vAccountLog.push_back(acctInfoLog);
	txundo.txHash = GetHash();
	return true;
}
bool CRewardTransaction::GetAddress(set<CKeyID> &vAddr, CAccountViewCache &view) {
	CKeyID keyId;
	if (account.type() == typeid(CRegID)) {
		if (!view.GetKeyId(account, keyId))
			return false;
		vAddr.insert(keyId);
	} else if (account.type() == typeid(CPubKey)) {
		CPubKey pubKey = boost::get<CPubKey>(account);
		if (!pubKey.IsFullyValid())
			return false;
		vAddr.insert(pubKey.GetKeyID());
	}
	return true;
}
string CRewardTransaction::ToString(CAccountViewCache &view) const {
	string str;
	CKeyID keyId;
	view.GetKeyId(account, keyId);
	CRegID regId;
	view.GetRegId(account, regId);
	str += strprintf("txType=%s, hash=%s, ver=%d, account=%s, keyid=%s, rewardValue=%ld\n", txTypeArray[nTxType], GetHash().ToString().c_str(), nVersion, regId.ToString(), keyId.GetHex(), rewardValue);
	return str;
}
bool CRewardTransaction::CheckTransction(CValidationState &state, CAccountViewCache &view) {
	return true;
}


bool CRegisterAppTx::ExecuteTx(int nIndex, CAccountViewCache &view,CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {
	CID id(regAcctId);
	CAccount acctInfo;
	CScriptDBOperLog operLog;
	if (!view.GetAccount(regAcctId, acctInfo)) {
		return state.DoS(100, ERRORMSG("ExecuteTx() : read regist addr %s account info error", HexStr(id.GetID())),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}
	CAccount acctInfoLog(acctInfo);
	uint64_t minusValue = llFees;
	if (minusValue > 0) {
		if(!acctInfo.OperateAccount(MINUS_FREE, minusValue))
			return state.DoS(100, ERRORMSG("ExecuteTx() : OperateAccount account regId=%s error", boost::get<CRegID>(regAcctId).ToString()),
					UPDATE_ACCOUNT_FAIL, "operate-account-failed");
		txundo.vAccountLog.push_back(acctInfoLog);
	}
	txundo.txHash = GetHash();

	CVmScript vmScript;
	CDataStream stream(script, SER_DISK, CLIENT_VERSION);
	try {
		stream >> vmScript;
	} catch (exception& e) {
		return state.DoS(100, ERRORMSG(("ExecuteTx() :intial() Unserialize to vmScript error:" + string(e.what())).c_str()),
				UPDATE_ACCOUNT_FAIL, "bad-query-scriptdb");
	}
	if(!vmScript.IsValid())
		return state.DoS(100, ERRORMSG("ExecuteTx() : vmScript invalid"), UPDATE_ACCOUNT_FAIL, "bad-query-scriptdb");

	CRegID regId(nHeight, nIndex);
	//create script account
	CKeyID keyId = Hash160(regId.GetVec6());
	CAccount account;
	account.keyID = keyId;
	account.regID = regId;
	//save new script content
	if(!scriptCache.SetScript(regId, script)){
		return state.DoS(100,
				ERRORMSG("ExecuteTx() : save script id %s script info error", regId.ToString()),
				UPDATE_ACCOUNT_FAIL, "bad-save-scriptdb");
	}
	if (!view.SaveAccountInfo(regId, keyId, account)) {
		return state.DoS(100,
				ERRORMSG("ExecuteTx() : create new account script id %s script info error",
						regId.ToString()), UPDATE_ACCOUNT_FAIL, "bad-save-scriptdb");
	}

	nRunStep = script.size();

	if(!operLog.vKey.empty()) {
		txundo.vScriptOperLog.push_back(operLog);
	}
	CUserID userId = acctInfo.keyID;
	if (!view.SetAccount(userId, acctInfo))
		return state.DoS(100, ERRORMSG("ExecuteTx() : write secure account info error"), UPDATE_ACCOUNT_FAIL,
				"bad-save-accountdb");
	return true;
}
bool CRegisterAppTx::UndoExecuteTx(int nIndex, CAccountViewCache &view, CValidationState &state, CTxUndo &txundo,
		int nHeight, CTransactionDBCache &txCache, CScriptDBViewCache &scriptCache) {
	CID id(regAcctId);
	CAccount account;
	CUserID userId;
	if (!view.GetAccount(regAcctId, account)) {
		return state.DoS(100, ERRORMSG("UndoUpdateAccount() : read regist addr %s account info error", HexStr(id.GetID())),
				UPDATE_ACCOUNT_FAIL, "bad-read-accountdb");
	}

	if(script.size() != 6) {

		CRegID scriptId(nHeight, nIndex);
		//delete script content
		if (!scriptCache.EraseScript(scriptId)) {
			return state.DoS(100, ERRORMSG("UndoUpdateAccount() : erase script id %s error", scriptId.ToString()),
					UPDATE_ACCOUNT_FAIL, "bad-save-scriptdb");
		}
		//delete account
		if(!view.EraseId(scriptId)){
			return state.DoS(100, ERRORMSG("UndoUpdateAccount() : erase script account %s error", scriptId.ToString()),
								UPDATE_ACCOUNT_FAIL, "bad-save-scriptdb");
		}
		CKeyID keyId = Hash160(scriptId.GetVec6());
		userId = keyId;
		if(!view.EraseAccount(userId)){
			return state.DoS(100, ERRORMSG("UndoUpdateAccount() : erase script account %s error", scriptId.ToString()),
								UPDATE_ACCOUNT_FAIL, "bad-save-scriptdb");
		}
//		LogPrint("INFO", "Delete regid %s app account\n", scriptId.ToString());
	}

	for(auto &itemLog : txundo.vAccountLog){
		if(itemLog.keyID == account.keyID) {
			if(!account.UndoOperateAccount(itemLog))
				return state.DoS(100, ERRORMSG("UndoUpdateAccount: UndoOperateAccount error, keyId=%s", account.keyID.ToString()),
						UPDATE_ACCOUNT_FAIL, "undo-account-failed");
		}
	}

	vector<CScriptDBOperLog>::reverse_iterator rIterScriptDBLog = txundo.vScriptOperLog.rbegin();
	for(; rIterScriptDBLog != txundo.vScriptOperLog.rend(); ++rIterScriptDBLog) {
		if(!scriptCache.UndoScriptData(rIterScriptDBLog->vKey, rIterScriptDBLog->vValue))
			return state.DoS(100,
					ERRORMSG("ExecuteTx() : undo scriptdb data error"), UPDATE_ACCOUNT_FAIL, "bad-save-scriptdb");
	}
	userId = account.keyID;
	if (!view.SetAccount(userId, account))
		return state.DoS(100, ERRORMSG("ExecuteTx() : write secure account info error"), UPDATE_ACCOUNT_FAIL,
				"bad-save-accountdb");
	return true;
}
bool CRegisterAppTx::GetAddress(set<CKeyID> &vAddr, CAccountViewCache &view) {
	CKeyID keyId;
	if (!view.GetKeyId(regAcctId, keyId))
		return false;
	vAddr.insert(keyId);
	return true;
}
string CRegisterAppTx::ToString(CAccountViewCache &view) const {
	string str;
	CKeyID keyId;
	view.GetKeyId(regAcctId, keyId);
	str += strprintf("txType=%s, hash=%s, ver=%d, accountId=%s, keyid=%s, llFees=%ld, nValidHeight=%d\n",
	txTypeArray[nTxType], GetHash().ToString().c_str(), nVersion,boost::get<CRegID>(regAcctId).ToString(), keyId.GetHex(), llFees, nValidHeight);
	return str;
}
bool CRegisterAppTx::CheckTransction(CValidationState &state, CAccountViewCache &view) {
	CAccount  account;
	if(!view.GetAccount(regAcctId, account)) {
		return state.DoS(100, ERRORMSG("CheckTransaction() : register app tx get registe account info error"), REJECT_INVALID,
				"bad-read-account-info");
	}

	if (!MoneyRange(llFees)) {
			return state.DoS(100, ERRORMSG("CheckTransaction() : register app tx fee out of range"), REJECT_INVALID,
					"bad-register-app-fee-toolarge");
	}
	uint64_t llFuel = ceil(script.size()/100) * GetFuelRate();
	if (llFuel < 1 * COIN) {
		llFuel = 1 * COIN;
	}

	if( llFees < llFuel) {
		return state.DoS(100, ERRORMSG("CheckTransaction() : register app tx fee too litter (actual:%lld vs need:%lld)", llFees, llFuel), REJECT_INVALID,
							"bad-register-app-fee-toolarge");
	}

	CAccount acctInfo;
	if (!view.GetAccount(boost::get<CRegID>(regAcctId), acctInfo)) {
		return state.DoS(100, ERRORMSG("CheckTransaction() :tx GetAccount falied"), REJECT_INVALID, "bad-getaccount");
	}
	if (!acctInfo.IsRegister()) {
		return state.DoS(100, ERRORMSG("CheckTransaction(): account have not registed public key"), REJECT_INVALID,
				"bad-no-pubkey");
	}
	uint256 signhash = SignatureHash();
	if (!CheckSignScript(signhash, signature, acctInfo.PublicKey)) {
		return state.DoS(100, ERRORMSG("CheckTransaction() :CheckSignScript failed"), REJECT_INVALID,
				"bad-signscript-check");
	}

	return true;
}


//bool CFund::IsMergeFund(const int & nCurHeight, int &nMergeType) const {
//	if (nCurHeight - nHeight > COINBASE_MATURITY) {
//		nMergeType = FREEDOM;  // Merget to Freedom;
//		return true;
//	}
//	return false;
//}
//Object CFund::ToJosnObj() const
//{
//	Object obj;
//	obj.push_back(Pair("value",     value));
//	obj.push_back(Pair("confirmed hight",     nHeight));
//	return obj;
//}
//string CFund::ToString() const {
//	string str;
//
//	str += strprintf("                value=%ld, nHeight=%d\n", value, nHeight);
//	return str;
////	return write_string(Value(ToJosnObj()),true);
//}
//
//string COperFund::ToString() const {
//	string str("");
//	string strOperType[] = { "NULL_OPER_TYPE", "ADD_FUND", "MINUS_FUND" };
//	str += strprintf("        list funds: operType=%s\n", strOperType[operType]);
//	str += fund.ToString();
//	return str;
//}

string CAccountLog::ToString() const {
	string str("");
	str += strprintf("    Account log: keyId=%d llValues=%lld nHeight=%lld nCoinDay=%lld\n",
			keyID.GetHex(), llValues, nHeight, nCoinDay);
	return str;
}

string CTxUndo::ToString() const {
	vector<CAccountLog>::const_iterator iterLog = vAccountLog.begin();
	string strTxHash("txHash:");
	strTxHash += txHash.GetHex();
	strTxHash += "\n";
	string str("  list account Log:\n");
	for (; iterLog != vAccountLog.end(); ++iterLog) {
		str += iterLog->ToString();
	}
	strTxHash += str;
	vector<CScriptDBOperLog>::const_iterator iterDbLog = vScriptOperLog.begin();
	string strDbLog(" list script db Log:\n");
	for	(; iterDbLog !=  vScriptOperLog.end(); ++iterDbLog) {
		strDbLog += iterDbLog->ToString();
	}
	strTxHash += strDbLog;
	return strTxHash;
}

bool CTxUndo::GetAccountOperLog(const CKeyID &keyId, CAccountLog &accountLog) {
	vector<CAccountLog>::iterator iterLog = vAccountLog.begin();
	for (; iterLog != vAccountLog.end(); ++iterLog) {
		if (iterLog->keyID == keyId) {
			accountLog = *iterLog;
			return true;
		}
	}
	return false;
}


//bool CAccount::CompactAccount(int nCurHeight) {
//	if (nCurHeight <= 0) {
//		return false;
//	}
//	return MergerFund(vRewardFund, nCurHeight);
//}
//bool CAccount::MergerFund(vector<CFund> &vFund, int nCurHeight) {
//	stable_sort(vFund.begin(), vFund.end(), greater<CFund>());
//	bool bHasMergd(false);
//	if(nCurHeight < nHeight) {
////		assert(0);
//		return false;
//	}
//	nCoinDay += llValues * ((int64_t)nCurHeight-(int64_t)nHeight);
//	vector<CFund>::reverse_iterator iterFund = vFund.rbegin();
//	for (; iterFund != vFund.rend();) {
//		if (nCurHeight - iterFund->nHeight > COINBASE_MATURITY) {
//			llValues += iterFund->value;
//			nCoinDay += iterFund->value * ((int64_t)nCurHeight-(int64_t)(iterFund->nHeight));
//			vFund.erase((iterFund++).base());
//			bHasMergd = true;
//		}
//		else {
//			break;
//		}
//	}
//	nHeight = nCurHeight;
//	if(nCoinDay > GetMaxCoinDay(nCurHeight)) {
//		nCoinDay = GetMaxCoinDay(nCurHeight);
//	}
//	return bHasMergd;
//}

bool CAccount::UndoOperateAccount(const CAccountLog & accountLog) {
//	LogPrint("undo_account", "after operate:%s\n", ToString());
	llValues = 	accountLog.llValues;
	nHeight = accountLog.nHeight;
	nCoinDay = accountLog.nCoinDay;
//	LogPrint("undo_account", "before operate:%s\n", ToString().c_str());
	return true;
}
void CAccount::ClearAccPos(int nCurHeight) {
	UpDateCoinDay(nCurHeight);
	nCoinDay = 0;
}
uint64_t CAccount::GetAccountPos(int nCurHeight){
	UpDateCoinDay(nCurHeight);
	return nCoinDay;
}
bool CAccount::UpDateCoinDay(int nCurHeight) {
	if(nCurHeight < nHeight)
		return false;
	nCoinDay += llValues * ((int64_t)nCurHeight-(int64_t)nHeight);
	nHeight = nCurHeight;
	if(nCoinDay > GetMaxCoinDay(nCurHeight)) {
		nCoinDay = GetMaxCoinDay(nCurHeight);
	}
	return true;
}
uint64_t CAccount::GetRawBalance() {
	return llValues;
}
Object CAccount::ToJosnObj() const
{
	using namespace json_spirit;
	Object obj;
	static const string fundTypeArray[] = { "NULL_FUNDTYPE", "FREEDOM", "REWARD_FUND", "FREEDOM_FUND"};
//	obj.push_back(Pair("height", chainActive.Height()));
	obj.push_back(Pair("Address",     keyID.ToAddress()));
	obj.push_back(Pair("KeyID",     keyID.ToString()));
	obj.push_back(Pair("RegID",     regID.ToString()));
	obj.push_back(Pair("PublicKey",  PublicKey.ToString()));
	obj.push_back(Pair("MinerPKey",  MinerPKey.ToString()));
	obj.push_back(Pair("FreeValues",     llValues));
	obj.push_back(Pair("CoinDays", nCoinDay));
	obj.push_back(Pair("UpdateHeight", nHeight));

	return obj;
}
string CAccount::ToString() const {
	string str;
	str += strprintf("regID=%s, keyID=%s, publicKey=%s, minerpubkey=%s, values=%ld updateHeight=%d coinDay=%lld\n",
	regID.ToString(), keyID.GetHex().c_str(), PublicKey.ToString().c_str(), MinerPKey.ToString().c_str(), llValues, nHeight, nCoinDay);
	return str;
	//return  write_string(Value(ToJosnObj()),true);
}

bool CAccount::IsMoneyOverflow(uint64_t nAddMoney) {
	if (!MoneyRange(nAddMoney))
		return false;

	uint64_t nTotalMoney = 0;
	nTotalMoney = llValues+nAddMoney;
	return MoneyRange(static_cast<int64_t>(nTotalMoney) );
}


bool CAccount::OperateAccount(OperType type, const uint64_t &value) {
//	LogPrint("op_account", "before operate:%s\n", ToString());
	if (keyID == uint160(0)) {
		assert(0);
	}
	if(!IsMoneyOverflow(value))
		return false;

	if (!value)
		return true;

	switch (type) {
	case ADD_FREE: {
		llValues += value;
		break;
	}
	case MINUS_FREE: {
		if(value > llValues)
			return false;
		uint64_t remainCoinDay = nCoinDay - value / llValues * nCoinDay;
		if(nCoinDay > llValues * SysCfg().GetIntervalPos()) {
			if(remainCoinDay < llValues*SysCfg().GetIntervalPos())
				remainCoinDay = llValues*SysCfg().GetIntervalPos();
		}
		nCoinDay = remainCoinDay;
		llValues -= value;
		break;
	}
	default:
		assert(0);
	}
//	LogPrint("op_account", "after operate:%s\n", ToString());
//	LogPrint("account", "oper log list:%s\n", accountOperLog.ToString());
	return true;
}
