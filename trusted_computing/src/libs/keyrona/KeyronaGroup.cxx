// Copyright (C) 2008, 2009 by Sirrix AG security technologies
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

/**
 * @file        KeyronaGroup.cxx
 * @brief       Implementation of the Keyrona group class.
 * @version     $Revision: 670 $
 * @date        Last modification at $Date: 2009-06-24 16:32:59 +0200 (Wed, 24 Jun 2009) $ by $Author: selhorst $
 */

#include <KeyronaGroup.hxx>

using namespace std;
using namespace utils;
using namespace keyrona;

//================================================================================
//
KeyronaGroup::KeyronaGroup(const string GroupID, KeyronaStorage &GroupStorage, KeyronaStorage &UserStorage) :
            myGroupSubjectIDs (NULL),
            myGroupID (GroupID),
            myGroupKeyfile (""),
            myGroupKeyUUID (""),
            myGroupKeyType (""),
            myGroupKey (NULL),
            myGroupStorage (GroupStorage),
            mySubjectStorage  (UserStorage)
{
    if (myGroupID.empty())
        throw InvalidGroup("KeyronaGroup: The supplied GroupID was empty!");

    // do we have such a group already?
    if (myGroupStorage.findSection(myGroupID))
    {
        debug << "KeyronaGroup[" + myGroupID + "|Constructor()]: Group '" + myGroupID + "' already exists, using that group." << endl;
        loadGroup();
        return;
    }

    // if not, create a new one
    if (myGroupID != KEYRONA_ADMIN_GROUP)
        myGroupID = "Group_" + myGroupID;
    debug << "KeyronaGroup[" + myGroupID + "|Constructor()]: Group '" + myGroupID + "' does not exist, creating new group." << endl;
    myGroupKeyfile = myKeyDirectory + KeyronaPathSeparator + KeyronaGroup_GroupKeyIdentifier  + KeyronaFileSeparator + myGroupID + KeyronaP15FileExtension;
        
    myGroupKeyUUID = generateUUID();
    myGroupKeyType = "blah";
        
    // generating random password for new key
    string myGroupKeyPassword = generateRandomString(KEYRONA_MINIMUM_PASSWORD_LENGTH);

    // creating new group key
    myGroupKey = new KeyronaKey(this, myGroupKeyPassword);
    // creating test vector
    ByteVector testVector = convertStringToByteVector(myGroupID);
    ByteVector testVectorByteVector = encryptForGroup(this, testVector);
    string myBase64EncodedTestVector = EncodeByteVectorToBASE64(testVectorByteVector);

    // storing group
    storeGroup();

    // storing test vector
    myGroupStorage.selectSection(myGroupID);
    myGroupStorage.selectSubSection(KeyronaGroup_GroupProperties);
    myGroupStorage.setEntry(KeyronaGroup_TestVector, myBase64EncodedTestVector);

    // check if this group is the admin group
    if (myGroupID == KEYRONA_ADMIN_GROUP)
    {
        debug << "KeyronaGroup[" + myGroupID + "|Constructor()]: Adminstrator group selected, encrypting group password for all available admins..." << endl;
        vector<StringPair> myAdmins = mySubjectStorage.findAllEntries(KeyronaSubject_SubjectAdmin, KeyronaSubject_TRUE);

        vector<utils::StringPair>::const_iterator Iterator;
        if (myAdmins.size())
        {
            Iterator=myAdmins.begin();
            while ( Iterator != myAdmins.end())
            {
                KeyronaSubject *mySubject = new KeyronaSubject (Iterator->first, mySubjectStorage);
                cout << "\tEncrypting group password for admin: " << mySubject->getMySubjectName() << " (" << mySubject->getMySubjectIDString() << ")" << endl;
                addSubjectToGroup(mySubject, myGroupKeyPassword);
                delete mySubject;
                Iterator++;
            }
        }
        else
        {
            deleteGroupByAdmin();
            throw InconsistentDatabase("KeyronaGroup[" + myGroupID + "|Constructor()]: no admins found! Please create an administrator first!");
        }
    }
    else
    {
        // ENCRYPT GROUPKEY PASSWORD FOR ADMINS!
        debug << "KeyronaGroup[" + myGroupID + "|Constructor()]: Encrypting group password for the admin group" << endl;
        KeyronaGroup myAdminGroup(KEYRONA_ADMIN_GROUP, GroupStorage, UserStorage);
        ByteVector passwordVector = convertStringToByteVector(myGroupKeyPassword);
        ByteVector passwordVectorByteVector = myAdminGroup.encryptForGroup(this, passwordVector);

        if (!passwordVector.size())
            throw EncryptionFailed("KeyronaGroup[" + myGroupID + "|Constructor()]: Encryption failed for admin group '" + KEYRONA_ADMIN_GROUP + "'!");

        // convert it into Base64
        string myBase64EncodedPassword = EncodeByteVectorToBASE64(passwordVectorByteVector);

        // select group keys section
        myGroupStorage.selectSection(myGroupID);
        myGroupStorage.selectSubSection(KeyronaGroup_GroupKeys);
        myGroupStorage.setEntry(KEYRONA_ADMIN_GROUP, myBase64EncodedPassword);
    }
        
};


//================================================================================
//
KeyronaGroup::~KeyronaGroup()
{
    
};

//================================================================================
//
void    KeyronaGroup::printGroup(bool verbose)
{
    cout << "GroupID: '" << myGroupID << "'" << endl;
    if (verbose)
    {
        cout << "\tGroupKeyfile: '" << myGroupKeyfile << "'" << endl;
        cout << "\tKey details:" << endl;
        // myGroupKey->printKeyInformation();
    }
};

//================================================================================
//
void KeyronaGroup::printSubjectsInGroup(bool verbose)
{
    vector<UInt32>::const_iterator Iterator;
    if (myGroupSubjectIDs.size())
    {
        cout << "Subjects in group: '" << myGroupID << "':" << endl;

        Iterator=myGroupSubjectIDs.begin();
        while ( Iterator != myGroupSubjectIDs.end())
        {
            KeyronaSubject *mySubject = new KeyronaSubject (*(Iterator), mySubjectStorage);
            mySubject->printSubject(verbose);
            delete mySubject;
            Iterator++;
        }
    }
    else
        cout << "No subjects in group '" << myGroupID << "'." << endl;
};

//================================================================================
//
void    KeyronaGroup::deleteGroup()
{
    debug << "KeyronaGroup[" << myGroupID << "|deleteGroup()] called!" << endl;
    if (myGroupID == KEYRONA_ADMIN_GROUP)
        throw InvalidGroup("The administrator group can not be deleted!");
    myGroupSubjectIDs.clear();
    myGroupStorage.deleteSection(myGroupID);
    myGroupID.clear();
};

//================================================================================
//
void    KeyronaGroup::deleteGroupByAdmin()
{
    debug << "KeyronaGroup[" << myGroupID << "|deleteGroupByAdmin()] called!" << endl;
    myGroupSubjectIDs.clear();
    myGroupStorage.deleteSection(myGroupID);
    myGroupID.clear();
};

//================================================================================
//
void    KeyronaGroup::addSubjectToGroup(KeyronaSubject *Subject, string &groupKeyPassword)
{
    debug << "KeyronaGroup[" << myGroupID << "|addSubjectToGroup()] called!" << endl;
    if (! Subject->getMySubjectID())
        throw InvalidSubjectID("KeyronaGroup["+myGroupID+"|addSubjectToGroup()]: Invalid SubjectID supplied!");

    // check if we are admin group and if subject is admin
    if ((myGroupID == KEYRONA_ADMIN_GROUP) && (! Subject->isAdmin()))
            throw InvalidSubjectID("KeyronaGroup["+myGroupID+"|addSubjectToGroup()]: You are not allowed to add non-admin subjects to the admin group!");
    // add subject to group
    UInt32 mySID = Subject->getMySubjectID();
    vector<UInt32>::const_iterator Iterator;
    Iterator=myGroupSubjectIDs.begin();
    while ( Iterator != myGroupSubjectIDs.end())
    {
        if (*(Iterator) == mySID)
        {
            debug << "KeyronaGroup["+myGroupID+"|addSubjectToGroup()]: Subject already in group!" << endl;
            return;
        }
        Iterator++;
    }

    debug << "KeyronaGroup["+myGroupID+"|addSubjectToGroup()]: generating testvector" << endl;
    // generate reference testVector
    vector<UInt8> referenceTestVector = convertStringToUInt8Vector(myGroupID);

    debug << "KeyronaGroup["+myGroupID+"|addSubjectToGroup()]: reading testvector from database" << endl;
    // get testVector from database
    myGroupStorage.selectSection(myGroupID);
    myGroupStorage.selectSubSection(KeyronaGroup_GroupProperties);
    string encryptedTestVector = myGroupStorage.getEntry(KeyronaGroup_TestVector);

    // decode testVector
    ByteVector decodedTestVector = DecodeBASE64StringToByteVector(encryptedTestVector);
	ByteVector decodedTestVectorByteVector;
	
    debug << "KeyronaGroup["+myGroupID+"|addSubjectToGroup()]: decrypting testvector from database with given password" << endl;
    // decrypted testVector
    cout <<" test2" << endl;
    try
    {
        decodedTestVectorByteVector = decryptByGroup(this, decodedTestVector, groupKeyPassword);
    }
    catch ( std::exception &e )
    {
        cout << "Error: could not add subject to group, since the given group key password was invalid!" << endl;
        throw DecryptionFailed("KeyronaGroup|addSubjectToGroup(): decryption of testvector failed!");
    }

    debug << "KeyronaGroup["+myGroupID+"|addSubjectToGroup()]: verifying decrypted testvector with reference testvector" << endl;
    if (decodedTestVectorByteVector != referenceTestVector)
        throw DecryptionFailed("KeyronaGroup|addSubjectToGroup(): decryption of testvector failed with given passphrase!");

    debug << "KeyronaGroup["+myGroupID+"|addSubjectToGroup()]: verification successful!" << endl;

    debug << "KeyronaGroup["+myGroupID+"|addSubjectToGroup()]: encrypting group key for new subject: '" << Subject->getMySubjectName() << "'" << endl;
    cout << "test" << endl;
    // encrypt group key password for subject
    cout <<" test4" << endl;
    				string test = Subject->getMySubjectKeyUUID();
				cout << test << endl;
    ByteVector passwordVector = convertStringToByteVector(groupKeyPassword);
    ByteVector passwordVectorByteVector = Subject->encryptForSubject(Subject, passwordVector);
    if (!passwordVector.size())
        throw EncryptionFailed("KeyronaGroup[" + myGroupID + "|Constructor()]: Encryption failed for admin '" + Subject->getMySubjectName() + "'!");
    cout <<" test5" << endl;

    // convert it into Base64
    string myBase64EncodedPassword = EncodeByteVectorToBASE64(passwordVectorByteVector);

    // store the encrypted key password for subejct
    // select keys section
    myGroupStorage.selectSection(myGroupID);
    myGroupStorage.selectSubSection(KeyronaGroup_SubjectKeys);
    myGroupStorage.setEntry(Subject->getMySubjectIDString(), myBase64EncodedPassword);

    // add subject to groups subject list
    myGroupSubjectIDs.push_back(mySID);

    // store subject List
    storeSubjectList();
};

//================================================================================
//
void    KeyronaGroup::deleteSubjectFromGroup(KeyronaSubject *Subject)
{
    debug << "KeyronaGroup[" << myGroupID << "|deleteSubjectFromGroup()] called!" << endl;

    if (! Subject->getMySubjectID())
        throw InvalidSubjectID("KeyronaGroup["+myGroupID+"|addSubjectToGroup()]: Invalid SubjectID supplied!");

    UInt32 mySubjectToDelete = Subject->getMySubjectID();

    vector<UInt32>::iterator Iterator;
    Iterator=myGroupSubjectIDs.begin();
    while ( Iterator != myGroupSubjectIDs.end())
    {
        if (*(Iterator) == mySubjectToDelete)
        {
            debug << "KeyronaGroup[" << myGroupID << "|deleteSubjectFromGroup()] Deleting subject '" << mySubjectToDelete << "'!" << endl;

            // deleting subject-encrypted groupkey
            myGroupStorage.selectSection(myGroupID);
            myGroupStorage.selectSubSection(KeyronaGroup_SubjectKeys);
            // delete encrypted group key for subject
            myGroupStorage.deleteEntry(Subject->getMySubjectIDString());
            // deleting subject from groups subject list
            myGroupSubjectIDs.erase(Iterator);
            storeSubjectList();
            return;
        }
        Iterator++;
    }
};

//================================================================================
//
bool    KeyronaGroup::isSubjectInGroup(KeyronaSubject *Subject)
{
    debug << "KeyronaGroup[" << myGroupID << "|isSubjectInGroup()] called!" << endl;

    if (! Subject->getMySubjectID())
        throw InvalidSubjectID("KeyronaGroup["+myGroupID+"|isSubjectInGroup()]: Invalid SubjectID supplied!");
    
    vector<UInt32>::const_iterator Iterator;
    if (myGroupSubjectIDs.size())
    {
        UInt32 SubjectToFind = Subject->getMySubjectID();
        Iterator=myGroupSubjectIDs.begin();
        while ( Iterator != myGroupSubjectIDs.end())
        {
            if (*(Iterator) == SubjectToFind)
                return true;
            Iterator++;
        }
    }
    return false;
};

//================================================================================
//
void KeyronaGroup::storeSubjectList()
{
    debug << "KeyronaGroup[" << myGroupID << "|storeSubjects()] called!" << endl;
    myGroupStorage.selectSection(myGroupID);
    myGroupStorage.deleteSubSection(KeyronaGroup_SubjectList);
    myGroupStorage.selectSubSection(KeyronaGroup_SubjectList);

    vector<UInt32>::const_iterator Iterator;
    Iterator=myGroupSubjectIDs.begin();

    while ( Iterator != myGroupSubjectIDs.end())
    {
        debug << "KeyronaGroup[" << myGroupID << "|storeSubjects()] Storing SubjectID: " << *(Iterator) << endl;
        myGroupStorage.setEntry(convertUInt32toString(*(Iterator)), convertUInt32toString(*(Iterator)));
        Iterator++;
    }
};

//================================================================================
//
void KeyronaGroup::loadSubjectList()
{
    debug << "KeyronaGroup[" << myGroupID << "|loadSubjects()] called!" << endl;
    myGroupStorage.selectSection(myGroupID);
    myGroupStorage.selectSubSection(KeyronaGroup_SubjectList);
    vector <string> myEntries = myGroupStorage.getAllEntries();

    vector<string>::const_iterator Iterator;
    Iterator=myEntries.begin();
    myGroupSubjectIDs.clear();

    while ( Iterator != myEntries.end())
    {
        debug << "KeyronaGroup[" << myGroupID << "|loadSubjects()] Loaded SubjectID: " << *(Iterator) << endl;
        myGroupSubjectIDs.push_back(convertStringtoUInt32(*(Iterator)));
        Iterator++;
    }
};

//================================================================================
//
void KeyronaGroup::storeGroup()
{
    debug << "KeyronaGroup[" << myGroupID << "|storeGroup()] called!" << endl;
    myGroupStorage.selectSection(myGroupID);
    myGroupStorage.selectSubSection(KeyronaGroup_GroupProperties);
    myGroupStorage.setEntry(KeyronaGroup_isGroup, KeyronaGroup_isGroup_true);
    myGroupStorage.setEntry(KeyronaGroup_GroupKeyfile, myGroupKeyfile);
	myGroupStorage.setEntry(KeyronaGroup_GroupKeyUUID, myGroupKeyUUID);
	myGroupStorage.setEntry(KeyronaGroup_GroupKeyType, myGroupKeyType);

    // assigning group and file permission
    assignFilePermission(myGroupKeyfile, KEYFILEPERMISSION);
    storeSubjectList();
};

//================================================================================
//
void KeyronaGroup::loadGroup()
{
    debug << "KeyronaGroup[" << myGroupID << "|loadGroup()] called!" << endl;
    myGroupStorage.selectSection(myGroupID);
    myGroupStorage.selectSubSection(KeyronaGroup_GroupProperties);
    myGroupKeyUUID = myGroupStorage.getEntry(KeyronaGroup_GroupKeyUUID);
    myGroupKeyType = myGroupStorage.getEntry(KeyronaGroup_GroupKeyType);
    myGroupKeyfile = myGroupStorage.getEntry(KeyronaGroup_GroupKeyfile);
    loadSubjectList();
};

//================================================================================
//
string  KeyronaGroup::getDecryptedGroupKey(KeyronaSubject *Subject, string &password)
{
    if (!Subject->getMySubjectID())
        throw InvalidSubjectID("KeyronaGroup|getDecryptedGroupKey(): Invalid Subject ID supplied!");

    myGroupStorage.selectSection(myGroupID);
    myGroupStorage.selectSubSection(KeyronaGroup_SubjectKeys);

    debug << "KeyronaGroup|getDecryptedGroupKey(): receiving BASE64-encoded key" << endl;
    string myBase64EncodedPassword = myGroupStorage.getEntry(Subject->getMySubjectIDString());
	cout << "seg" << endl;

    debug << "KeyronaGroup|getDecryptedGroupKey(): decoding BASE64-encoded key" << endl;
    ByteVector myDecode = DecodeBASE64StringToByteVector(myBase64EncodedPassword);
    	cout << "seg2" << endl;

    debug << "KeyronaGroup|getDecryptedGroupKey(): decrypting key" << endl;
    ByteVector blub = Subject->decryptBySubject(Subject, myDecode, password);
    	cout << "seg00" << endl;

    if (! myDecode.size() )
        throw DecryptionFailed("KeyronaGroup|getDecryptedGroupKey(): decryption failed for subject '" + Subject->getMySubjectName() + "'!");

    return convertByteVector2String(blub);
};

//================================================================================
//
string  KeyronaGroup::getDecryptedGroupKeyByAdmin(string &AdminKeyPassword)
{
    if (myGroupID == KEYRONA_ADMIN_GROUP)
        throw InvalidGroup("This operation is not allowed by the admin group!");

    debug << "KeyronaGroup[" + myGroupID + "|getDecryptedGroupKeyByAdmin()]: Decrypting group password by the admin group" << endl;

    // selecting group keys section
    myGroupStorage.selectSection(myGroupID);
    myGroupStorage.selectSubSection(KeyronaGroup_GroupKeys);

    // retrieving group key password, that is encrypted for the admin group
    string myBase64EncodedGroupKeyPassword = myGroupStorage.getEntry(KEYRONA_ADMIN_GROUP);

    // converting into vector<UInt8>
    ByteVector decodedGroupKeyPassword = DecodeBASE64StringToByteVector(myBase64EncodedGroupKeyPassword);

    // opening admin group
    KeyronaGroup myAdminGroup(KEYRONA_ADMIN_GROUP, myGroupStorage, mySubjectStorage);
    // decrypting group key password with admin key
    decodedGroupKeyPassword = myAdminGroup.decryptByGroup(this, decodedGroupKeyPassword, AdminKeyPassword);

    return convertByteVector2String(decodedGroupKeyPassword);
};
//================================================================================
//
ByteVector  KeyronaGroup::encryptForGroup(KeyronaGroup *Group, ByteVector &toEncrypt)
{
    //if ((myGroupKey) && (myGroupKey->isValid()))
        return myGroupKey->encrypt(NULL, this, toEncrypt);
    //throw InvalidKey("KeyronaGroup|encryptForGroup(): Invalid key for group '" + myGroupID + "'");
};

//================================================================================
//
ByteVector  KeyronaGroup::decryptByGroup(KeyronaGroup *Group, ByteVector &toDecrypt, string &myPassword)
{
	cout << myGroupKey << endl;
    //if ((myGroupKey) && (myGroupKey->isValid()))
        return myGroupKey->decrypt(NULL, this, toDecrypt, myPassword);
    //throw InvalidKey("KeyronaGroup|decryptByGroup(): Invalid key for group '" + myGroupID + "'");
};

void KeyronaGroup::addMessageForGroupMembers(string message)
{
    if (! myGroupSubjectIDs.size())
        return;

    if ( ! message.size())
        throw NoMessage("KeyronaGroup|addMessageForGroupMembers(): Empty message supplied!");

    // convert our subjectIDs into subjects
    vector<UInt32>::const_iterator Iterator;
    Iterator=myGroupSubjectIDs.begin();
    while ( Iterator != myGroupSubjectIDs.end())
    {
        KeyronaSubject *mySubject = new KeyronaSubject(*(Iterator), mySubjectStorage);
        mySubject->addMessageForUser(message);
        printToSyslog("KEYRONA: Group '" + myGroupID + "': Sending group message => '" + message + "'");
        delete mySubject;
        Iterator++;
    }
};
