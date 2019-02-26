1. Java_me_mi_mp3_util_LameUtil_encodeFile 这个方法是用于双声道的pcm转mp3的(lame_encode_buffer_interleaved) 2019/2/26
	LameUtil.init(16000, 2, 16000, 32, 7);
	LameUtil.encodeFile("*.pcm", "*.mp3");
