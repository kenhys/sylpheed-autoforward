メール転送プラグイン
================================================================================

このプラグインは受信したメールをそのままあらかじめ指定したアドレスへと転送します。

動作環境
--------------------------------------------------------------------------------

Windows XP SP3以降

インストール方法
--------------------------------------------------------------------------------

1. dllを配置

autoforward.dllをSylpheedのプラグインディレクトリへとコピーします。
Sylpheedのzipアーカイブを利用している場合には、pluginsディレクトリへ配置するだけです。

インストーラにて導入している場合などは%APPDATA%/Sylpheed/plugins以下に配置します。

XP環境では%APPDATA%/Sylpheed/pluginsは以下の場所になります。

C:\Documents and Settings\(ユーザ名)\Application Data\Sylpheed\plugins

Windows 7環境では%APPDATA%/Sylpheed/pluginsは以下の場所になります。
C:/Users/(ユーザ名)/AppData/Roaming/Sylpheed/plugins

2. .moファイルの配置

autoforward.moをsylpheedディレクトリのlib/locale/ja/LC_MESSAGES/autoforward.moとしてコピーします

使い方
--------------------------------------------------------------------------------

[ツール]-[メール転送設定]を選択すると設定ダイアログが表示されます。
設定ダイアログでは以下の項目を指定することができます。


[転送先メールアドレス]
転送先のメールアドレスを指定します。

[メール転送を起動時に有効化]
Sylpheedを起動したときに転送を有効にしたい場合にはチェックを有効にします。

[転送条件 - すべてのメールを転送する]
受信したメールをすべて転送します。

[転送条件 - 指定フォルダ内のメールを転送]
指定したフォルダに振り分けられたメールを転送します。
[追加]ボタンをクリックして転送対象にするフォルダを選択してください。
追加したフォルダはリストに表示されます。
対象からはずすには表示されているフォルダを選択し[削除]ボタンをクリックして下さい。


転送機能はステータスバーのアイコンをクリックして有効または無効を切り替えることができます。
メールの自動チェックを有効にしていれば、上記でメールが届くたびに転送を行うことができます。

注意事項
--------------------------------------------------------------------------------

デフォルトアカウントのメールアドレスを転送先に指定しないでください。
受信 -> 転送 -> 受信とループします。

メールを手動で移動したときも転送されます。

