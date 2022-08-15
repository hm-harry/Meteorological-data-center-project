# .bash_profile

# Get the aliases and functions
if [ -f ~/.bashrc ]; then
        . ~/.bashrc
fi

# User specific environment and startup programs

PATH=$PATH:$HOME/.local/bin:$HOME/bin

export PATH

export ORACLE_BASE=/oracle/base
export ORACLE_HOME=/oracle/home
# 以下的ORACLE_SID，snoracl是字母，11是数字，g是字母。
export ORACLE_SID=snorcl11g
export NLS_LANG='Simplified Chinese_China.AL32UTF8'
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ORACLE_HOME/lib:/usr/lib:.
export PATH=$PATH:$HOME/bin:$ORACLE_HOME/bin:.

alias sqlplus='rlwrap sqlplus'
