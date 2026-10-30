package com.firebasehelper;

import android.app.Activity;
import android.content.Intent;
import android.util.Log;

import com.facebook.FacebookSdk;
import com.facebook.appevents.AppEventsLogger;
import com.facebook.AccessToken;
import com.facebook.CallbackManager;
import com.facebook.FacebookCallback;
import com.facebook.FacebookException;
import com.facebook.login.LoginManager;
import com.facebook.login.LoginResult;
import com.facebook.login.widget.LoginButton;

import java.util.Arrays;

public class FirebaseFacebookLogin {
    private static final String TAG = "FirebaseFacebook";
    private Activity mActivity;
    private CallbackManager mCallbackManager;

    public FirebaseFacebookLogin(Activity activity){
        mActivity = activity;

        mCallbackManager = CallbackManager.Factory.create();
        LoginManager.getInstance().registerCallback(mCallbackManager,
                new FacebookCallback<LoginResult>() {
                    @Override
                    public void onSuccess(LoginResult loginResult) {
                        nativeFacebookSignInResult(true, loginResult.getAccessToken().getToken());
                        Log.d(TAG, "Facebook LoginSuccess:"+ loginResult.getAccessToken().getToken() );
                    }

                    @Override
                    public void onCancel() {
                        nativeFacebookSignInResult(false, "");
                        Log.d(TAG, "Facebook LoginCancel");
                    }

                    @Override
                    public void onError(FacebookException exception) {
                        nativeFacebookSignInResult(false, "");
                        Log.d(TAG, "Facebook LoginException:" + exception);
                    }
                });
    }

    public void doFacebookSignIn(){
        AccessToken accessToken = AccessToken.getCurrentAccessToken();
        boolean isLoggedIn = accessToken != null && !accessToken.isExpired();
        if(isLoggedIn){
            Log.d(TAG, "FacebookLogin Success:" + accessToken.getToken() );
            nativeFacebookSignInResult(true, accessToken.getToken() );
            return;
        }

        LoginManager.getInstance().logInWithReadPermissions(mActivity, Arrays.asList("public_profile", "email"));
    }

    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        mCallbackManager.onActivityResult(requestCode, resultCode, data);
    }

    public native void nativeFacebookSignInResult(boolean success, String accessToken);
}
