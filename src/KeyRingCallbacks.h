/*  ---------------------------------------------------------
               __  __            _
              |  \/  |_   _ _ __| |_   _ _ __
              | |\/| | | | | '__| | | | | '_ \
              | |  | | |_| | |  | | |_| | | | |
              |_|  |_|\__, |_|  |_|\__, |_| |_|
                      |___/        |___/
    ---------------------------------------------------------

    Project:  Myrlyn Package Manager GUI
    Copyright (c) Stefan Hundhammer <Stefan.Hundhammer@gmx.de>
    License:  GPL V2 - See file LICENSE for details.

 */


#ifndef KeyRingCallbacks_h
#define KeyRingCallbacks_h

#include <iostream>        // cerr
#include <zypp/KeyRing.h>
#include <zypp/RepoInfo.h>

#include "RepoGpgKeyImportDialog.h"


//
// Classes for handling GPG keys in the libzypp key ring
//

typedef zypp::KeyRingReport::KeyTrust  ZyppKeyTrust;
// KEY_DONT_TRUST,
// KEY_TRUST_TEMPORARILY,
// KEY_TRUST_AND_IMPORT


struct KeyRingReceiveCallback:
    public zypp::callback::ReceiveReport<zypp::KeyRingReport>
{
    virtual ZyppKeyTrust askUserToAcceptKey( const zypp::PublicKey  & key,
                                             const zypp::KeyContext & context ) override
        {
#if 0
            std::cerr << "** Untrusted key **"
                      << "\nFingerprint: "  << zypp::str::gapify( key.fingerprint(), 4 )
                      << "\nRepo: " << context.repoInfo().name()
                      << "\nURL:  " << context.repoInfo().url()
                      << std::endl;
#endif
            RepoGpgKeyImportDialog dialog( key, context.repoInfo() );
            int result = dialog.exec();

            return result == QDialog::Accepted ?
                ZyppKeyTrust::KEY_TRUST_AND_IMPORT :
                ZyppKeyTrust::KEY_DONT_TRUST;
        }


    virtual bool askUserToAcceptUnsignedFile( const std::string      & file,
                                              const zypp::KeyContext & context ) override
        {
            std::cerr << "Untrusted file " << file << std::endl;
            return false; // Don't continue
        }


    virtual bool askUserToAcceptUnknownKey( const std::string      & file,
                                            const std::string      & id,
                                            const zypp::KeyContext & context ) override
        {
            std::cerr << "Untrusted unknown file with unknown key " << file << std::endl;
            return false; // Don't continue
        }


    virtual bool askUserToAcceptVerificationFailed( const std::string      & file,
                                                    const zypp::PublicKey  & key,
                                                    const zypp::KeyContext & context ) override
        {
            std::cerr << "Verification of " << file << " with key " << key << " failed" << std::endl;
            return false; // Don't continue
        }
};


class KeyRingCallbacks
{
public:

    /**
     * Constructor. This automatically connects the callbacks.
     **/
    KeyRingCallbacks();

    /**
     * Constructor. This automatically disconnects the callbacks.
     **/
    virtual ~KeyRingCallbacks();


protected:

    KeyRingReceiveCallback _keyRingReceiveCallback;
};



#endif // KeyRingCallbacks_h
