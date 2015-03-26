// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Dacrs developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "walletdb.h"

#include "base58.h"
#include "protocol.h"
#include "serialize.h"
#include "sync.h"
#include "wallet.h"
#include "tx.h"

#include <fstream>
#include <algorithm>
#include <iterator>
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost;


//
// CWalletDB
//




bool ReadKeyValue(CWallet* pwallet, CDataStream& ssKey, CDataStream& ssValue,string& strType, string& strErr)
{
	try {
		// Unserialize
		// Taking advantage of the fact that pair serialization
		// is just the two items serialized one after the other
		ssKey >> strType;

		if (strType == "tx") {
			uint256 hash;
			ssKey >> hash;
			std::shared_ptr<CBaseTransaction> pBaseTx; //= make_shared<CContractTransaction>();
			ssValue >> pBaseTx;
			if (pBaseTx->GetHash() == hash) {
				if (pwallet != NULL)
					pwallet->UnConfirmTx[hash] = pBaseTx->GetNewInstance();
			} else {
				strErr = "Error reading wallet database: tx corrupt";
				return false;
			}

		} else if (strType == "keystore") {
			CKeyStoreValue vKeyStore;
			CKeyID cKeyid;
			ssKey >> cKeyid;
			ssValue >> vKeyStore;
			if (vKeyStore.SelfCheck() != true || cKeyid != vKeyStore.GetCKeyID()) {
				strErr = "Error reading wallet database: keystore corrupt";
				return false;
			}
			if (pwallet != NULL)
				pwallet->mKeyPool[cKeyid] = std::move(vKeyStore);

		} else if (strType == "blocktx") {
			uint256 hash;
			CAccountTx atx;
			CKeyID cKeyid;
			ssKey >> hash;
			ssValue >> atx;
			if (pwallet != NULL)
				pwallet->mapInBlockTx[hash] = std::move(atx);
		} else if (strType == "defaultkey") {
			if (pwallet != NULL)
				ssValue >> pwallet->vchDefaultKey;
		} else if (strType != "version" && "minversion" != strType) {
			assert(0);
		}
	} catch (...) {
		return false;
	}
	return true;
}



DBErrors CWalletDB::LoadWallet(CWallet* pwallet)
{

	  pwallet->vchDefaultKey = CPubKey();
	    bool fNoncriticalErrors = false;
	    DBErrors result = DB_LOAD_OK;

	    try {
	        LOCK(pwallet->cs_wallet);
	          if (GetMinVersion() > CLIENT_VERSION)
	                return DB_TOO_NEW;


	        // Get cursor
	        Dbc* pcursor = GetCursor();
	        if (!pcursor)
	        {
	            LogPrint("INFO","Error getting wallet database cursor\n");
	            return DB_CORRUPT;
	        }

	        while (true)
	        {
	            // Read next record
	            CDataStream ssKey(SER_DISK, CLIENT_VERSION);
	            CDataStream ssValue(SER_DISK, CLIENT_VERSION);
	            int ret = ReadAtCursor(pcursor, ssKey, ssValue);
	            if (ret == DB_NOTFOUND)
	                break;
	            else if (ret != 0)
	            {
	                LogPrint("INFO","Error reading next record from wallet database\n");
	                return DB_CORRUPT;
	            }

	            // Try to be tolerant of single corrupt records:
	            string strType, strErr;
	            if (!ReadKeyValue(pwallet, ssKey, ssValue, strType, strErr))
	            {
	                // losing keys is considered a catastrophic error, anything else
	                // we assume the user can live with:
//	                if (IsKeyType(strType))
//	                    result = DB_CORRUPT;
				if (strType == "keystore"){
					return DB_CORRUPT;
				}else
	                {
	                    // Leave other errors alone, if we try to fix them we might make things worse.
	                    fNoncriticalErrors = true; // ... but do warn the user there is something wrong.
//	                    if (strType == "acctx")
//	                        // Rescan if there is a bad transaction record:
//	                        SoftSetBoolArg("-rescan", true);
	                }
	            }
	            if (!strErr.empty())
	                LogPrint("INFO","%s\n", strErr);
	        }
	        pcursor->close();
	    }
	    catch (boost::thread_interrupted) {
	        throw;
	    }
	    catch (...) {
	        result = DB_CORRUPT;
	    }

	    if (fNoncriticalErrors && result == DB_LOAD_OK)
	        result = DB_NONCRITICAL_ERROR;

	    // Any wallet corruption at all: skip any rewriting or
	    // upgrading, we don't want to make it worse.
	    if (result != DB_LOAD_OK)
	        return result;

	    LogPrint("INFO","nFileVersion = %d\n", GetMinVersion());



//	    // nTimeFirstKey is only reliable if all keys have metadata
//	    if ((wss.nKeys + wss.nCKeys) != wss.nKeyMeta)
//	        pwallet->nTimeFirstKey = 1; // 0 would be considered 'no value'
//
//	    for (auto hash : wss.vWalletUpgrade)
//	        WriteAccountTx(hash, pwallet->mapWalletTx[hash]);

//	    // Rewrite encrypted wallets of versions 0.4.0 and 0.5.0rc:
//	    if (wss.fIsEncrypted && (wss.nFileVersion == 40000 || wss.nFileVersion == 50000))
//	        return DB_NEED_REWRITE;

	    if ( GetMinVersion()< CLIENT_VERSION) // Update
	    	WriteVersion( GetMinVersion());

	if (pwallet->mKeyPool.size() == 0) {
		CKey mCkey;
		mCkey.MakeNewKey();
		if (!pwallet->AddKey(mCkey)) {
			throw runtime_error("add key failed ");
		}
	}

	  return result;



//    pwallet->vchDefaultKey = CPubKey();
////    bool fNoncriticalErrors = false;
//    DBErrors result = DB_LOAD_OK;
////
////    try {
////        LOCK(pwallet->cs_wallet);
////    	leveldb::Iterator *pcursor = db.NewIterator();
////    	// Load mapBlockIndex
////       	pcursor->SeekToFirst();
////    	while (pcursor->Valid()) {
////    		boost::this_thread::interruption_point();
////    		try {
////    			leveldb::Slice slKey = pcursor->key();
////    			CDataStream ssKey(slKey.data(), slKey.data() + slKey.size(), SER_DISK, CLIENT_VERSION);
////    			leveldb::Slice slValue = pcursor->value();
////    			CDataStream ssValue(slValue.data(), slValue.data() + slValue.size(), SER_DISK, CLIENT_VERSION);
////      		     // Try to be tolerant of single corrupt records:
////       		      string strType, strErr;
////    			if(!ReadKeyValue(pwallet,ssKey,ssValue,strType,strErr))
////    				{
////    				   if (strType == "keystore")
////    					 	delete pcursor;
////    				    return  DB_CORRUPT;
////    				};
////    			pcursor->Next();
////    			}
////    	 catch (std::exception &e) {
////    			delete pcursor;
////    			 ERRORMSG("%s : Deserialize or I/O error - %s", __func__, e.what());
////    			 return DB_CORRUPT;
////    		}
////    	}
////    	delete pcursor;
////
////    }
////    catch (boost::thread_interrupted) {
////        throw;
////    }
////    catch (...) {
////        result = DB_CORRUPT;
////    }
////    /// if the wallet not address ,create a new address
////    if(pwallet->mKeyPool.size() == 0){
////        CKey  mCkey;
////        mCkey.MakeNewKey();
////         if (!pwallet->AddKey(mCkey)) {
////       		throw runtime_error("add key failed ");
////       	}
////    }
////
//   return result;

}


//
// Try to (very carefully!) recover wallet.dat if there is a problem.
//
bool CWalletDB::Recover(CDBEnv& dbenv, string filename, bool fOnlyKeys)
{
    // Recovery procedure:
    // move wallet.dat to wallet.timestamp.bak
    // Call Salvage with fAggressive=true to
    // get as much data as possible.
    // Rewrite salvaged data to wallet.dat
    // Set -rescan so any missing transactions will be
    // found.
    int64_t now = GetTime();
    string newFilename = strprintf("wallet.%d.bak", now);

    int result = dbenv.dbenv->dbrename(NULL, filename.c_str(), NULL,
                                      newFilename.c_str(), DB_AUTO_COMMIT);
    if (result == 0)
        LogPrint("INFO","Renamed %s to %s\n", filename, newFilename);
    else
    {
        LogPrint("INFO","Failed to rename %s to %s\n", filename, newFilename);
        return false;
    }

    vector<CDBEnv::KeyValPair> salvagedData;
    bool allOK = dbenv.Salvage(newFilename, true, salvagedData);
    if (salvagedData.empty())
    {
        LogPrint("INFO","Salvage(aggressive) found no records in %s.\n", newFilename);
        return false;
    }
    LogPrint("INFO","Salvage(aggressive) found %u records\n", salvagedData.size());

    bool fSuccess = allOK;
    boost::scoped_ptr<Db> pdbCopy(new Db(dbenv.dbenv, 0));
    int ret = pdbCopy->open(NULL,               // Txn pointer
                            filename.c_str(),   // Filename
                            "main",             // Logical db name
                            DB_BTREE,           // Database type
                            DB_CREATE,          // Flags
                            0);
    if (ret > 0)
    {
        LogPrint("INFO","Cannot create database file %s\n", filename);
        return false;
    }
    DbTxn* ptxn = dbenv.TxnBegin();
    for (auto& row : salvagedData)
    {
        if (fOnlyKeys)
        {
            CDataStream ssKey(row.first, SER_DISK, CLIENT_VERSION);
            CDataStream ssValue(row.second, SER_DISK, CLIENT_VERSION);
            string strType, strErr;
            bool fReadOK = ReadKeyValue(NULL, ssKey, ssValue,
                                         strType, strErr);
            if (strType != "keystore")
                continue;
            if (!fReadOK)
            {
                LogPrint("INFO","WARNING: CWalletDB::Recover skipping %s: %s\n", strType, strErr);
                continue;
            }
        }
        Dbt datKey(&row.first[0], row.first.size());
        Dbt datValue(&row.second[0], row.second.size());
        int ret2 = pdbCopy->put(ptxn, &datKey, &datValue, DB_NOOVERWRITE);
        if (ret2 > 0)
            fSuccess = false;
    }
    ptxn->commit(0);
    pdbCopy->close(0);

    return fSuccess;
}

bool CWalletDB::Recover(CDBEnv& dbenv, string filename)
{
    return CWalletDB::Recover(dbenv, filename, false);
}



bool CWalletDB::WriteBlockTx(const uint256 &hash, const CAccountTx& atx)
{
	nWalletDBUpdated++;
	return Write(make_pair(string("blocktx"), hash), atx);
}
bool CWalletDB::EraseBlockTx(const uint256 &hash)
{
	nWalletDBUpdated++;
	return Erase(make_pair(string("blocktx"), hash));
}

bool CWalletDB::WriteKeyStoreValue(const CKeyID &keyId, const CKeyStoreValue& KeyStoreValue)
{
	nWalletDBUpdated++;
	return Write(make_pair(string("keystore"), keyId), KeyStoreValue,true);
}

bool CWalletDB::EraseKeyStoreValue(const CKeyID& keyId) {
	nWalletDBUpdated++;
	return Erase(make_pair(string("keystore"), keyId));
}




bool CWalletDB::WriteUnComFirmedTx(const uint256& hash, const std::shared_ptr<CBaseTransaction>& tx) {
	nWalletDBUpdated++;
	return Write(make_pair(string("tx"), hash),tx);
}


bool CWalletDB::EraseUnComFirmedTx(const uint256& hash) {
	nWalletDBUpdated++;
	return Erase(make_pair(string("tx"), hash));
}

bool CWalletDB::WriteVersion(const int version) {
	nWalletDBUpdated++;
	return Erase(make_pair(string("version"), version));
}
int CWalletDB::GetVersion(void) {
	int verion;
	return Read(string("version"),verion);
}
bool CWalletDB::WriteMinVersion(const int version) {
	nWalletDBUpdated++;
	return Erase(make_pair(string("minversion"), version));
}
int CWalletDB::GetMinVersion(void) {
	int verion;
	return Read(string("minversion"),verion);
}


bool CWalletDB::WriteMasterKey(const CMasterKey& kMasterKey)
{
    nWalletDBUpdated++;
    return Write(string("mkey"), kMasterKey, true);
}

bool CWalletDB::EraseMasterKey() {
    nWalletDBUpdated++;
    return Erase(string("mkey"));
}

CWalletDB::CWalletDB(const string& strFilename):CDB((GetDataDir() / strFilename).string(),"cr+")
{
	nWalletDBUpdated = 0 ;
}
unsigned int CWalletDB::nWalletDBUpdated = 0;

//extern CDBEnv bitdb;
void ThreadFlushWalletDB(const string& strFile)
{
    // Make this thread recognisable as the wallet flushing thread
    RenameThread("bitcoin-wallet");
    static bool fOneThread;
    if (fOneThread)
        return;
    fOneThread = true;
    if (!SysCfg().GetBoolArg("-flushwallet", true))
        return;

    unsigned int nLastSeen = CWalletDB::nWalletDBUpdated;
    unsigned int nLastFlushed = CWalletDB::nWalletDBUpdated;
    int64_t nLastWalletUpdate = GetTime();
    while (true)
    {
        MilliSleep(500);

        if (nLastSeen != CWalletDB::nWalletDBUpdated)
        {
            nLastSeen = CWalletDB::nWalletDBUpdated;
            nLastWalletUpdate = GetTime();
        }

        if (nLastFlushed != CWalletDB::nWalletDBUpdated && GetTime() - nLastWalletUpdate >= 2)
        {
            TRY_LOCK(bitdb.cs_db,lockDb);
            if (lockDb)
            {
                // Don't do this if any databases are in use
                int nRefCount = 0;
                map<string, int>::iterator mi = bitdb.mapFileUseCount.begin();
                while (mi != bitdb.mapFileUseCount.end())
                {
                    nRefCount += (*mi).second;
                    mi++;
                }

                if (nRefCount == 0)
                {
                    boost::this_thread::interruption_point();
                    map<string, int>::iterator mi = bitdb.mapFileUseCount.find(strFile);
                    if (mi != bitdb.mapFileUseCount.end())
                    {
                        LogPrint("db", "Flushing wallet.dat\n");
                        nLastFlushed = CWalletDB::nWalletDBUpdated;
                        int64_t nStart = GetTimeMillis();

                        // Flush wallet.dat so it's self contained
                        bitdb.CloseDb(strFile);
                        bitdb.CheckpointLSN(strFile);

                        bitdb.mapFileUseCount.erase(mi++);
                        LogPrint("db", "Flushed wallet.dat %dms\n", GetTimeMillis() - nStart);
                    }
                }
            }
        }
    }
}