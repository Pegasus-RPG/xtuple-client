#xslt with working the space handler
from lxml import etree
import urllib.request
from urllib.parse import quote
import re

#Used in translate function to format the output translation to match the source text format in the xml file. There was a problem with google translate adding/removing spaces before/after &amp;, &gt;, and &lt;.
#Google translate was also changing the encoding(i.e. &lt; to u\003c)
def space_handler(source, translated):
    #SECTION 1: INITIALIZING VARIABLES
    index_tracker = [0]
    partial_strings = []
    output_string = ''

    amp_count = translated.count('&amp;')

    # numregex1 = re.compile(r'&#[0-9]{2};')
    # numregex2 = re.compile(r'&#[0-9]{3};')

    gt_count_sr = source.count('&gt;')
    lt_count_sr = source.count('&lt;')

    lt_count_tr = translated.count('&lt;')
    ul_count_tr = translated.count('\\u003c')

    gt_count_tr = translated.count('&gt;')
    ug_count_tr = translated.count('\\u003e')


    index_dict_tr = {}
    index_dict_sr = {}
    index_list_tr = []
    index_list_sr = []

    should_chop = False

    #SECTION 2: CREATE 2 DICTIONARIES THAT CONTAIN WHAT TYPE OF INDEX EACH IMPORTANT INDEX IS
    ignore_counter = 0
    ignore_counter2 = 0
    for i in range(0, amp_count):
        holder = translated
        for j in range(0,ignore_counter+1):
            if j == ignore_counter:
                index_dict_tr[holder.index('&amp;')] = 'a'
                ignore_counter += 1
            else:
                holder = holder.replace('&amp;', 'filll',1)
        holder = source
        for z in range(0,ignore_counter2+1):
            if z == ignore_counter2:
                index_dict_sr[holder.index('&amp;')] = 'a'
                ignore_counter2+= 1
            else:
                holder = holder.replace('&amp;', 'filll', 1)

    # # mo1 = numregex1.findall(translated)
    # num_indexes = [m.start(0) for m in re.finditer(numregex1, translated)]
    # for index in num_indexes:
    #     index_dict_tr[index] = 'n1'
    # num_indexes = [m.start(0) for m in re.finditer(numregex1, source)]
    # for index in num_indexes:
    #     index_dict_sr[index] = 'n1'
    #
    # # mo2 = numregex2.findall(translated)
    # num_indexes = [m.start(0) for m in re.finditer(numregex2, translated)]
    # for index in num_indexes:
    #     index_dict_tr[index] = 'n2'
    # num_indexes = [m.start(0) for m in re.finditer(numregex2, source)]
    # for index in num_indexes:
    #     index_dict_sr[index] = 'n2'

    ignore_counter2 = 0
    for i in range(0, gt_count_sr):
        holder = source
        for z in range(0,ignore_counter2+1):
            if z == ignore_counter2:
                index_dict_sr[holder.index('&gt;')] = 'ga'
                ignore_counter2+= 1
            else:
                holder = holder.replace('&gt;', 'fill', 1)

    ignore_counter = 0
    for i in range(0, gt_count_tr):
        holder = translated
        for j in range(0, ignore_counter+1):
            if j == ignore_counter:
                index_dict_tr[holder.index('&gt;')] = 'ga'
                ignore_counter += 1
            else:
                holder = holder.replace('&gt;', 'fill', 1)

    ignore_counter = 0
    for i in range(0,ug_count_tr):
        holder = translated
        for j in range(0, ignore_counter+1):
            if j == ignore_counter:
                index_dict_tr[holder.index('\\u003e')] = 'gu'
                ignore_counter += 1
            else:
                holder = holder.replace('\\u003e', 'fillll', 1)

    ignore_counter2 = 0
    for i in range(0, lt_count_sr):
        holder = source
        for z in range(0,ignore_counter2+1):
            if z == ignore_counter2:
                index_dict_sr[holder.index('&lt;')] = 'la'
                ignore_counter2+= 1
            else:
                holder = holder.replace('&lt;', 'fill', 1)

    ignore_counter = 0
    for i in range(0, lt_count_tr):
        holder = translated
        for j in range(0, ignore_counter+1):
            if j == ignore_counter:
                index_dict_tr[holder.index('&lt;')] = 'la'
                ignore_counter += 1
            else:
                holder = holder.replace('&lt;', 'fill', 1)

    ignore_counter = 0
    for i in range(0,ul_count_tr):
        holder = translated
        for j in range(0, ignore_counter+1):
            if j == ignore_counter:
                index_dict_tr[holder.index('\\u003c')] = 'lu'
                ignore_counter += 1
            else:
                holder = holder.replace('\\u003c', 'fillll', 1)



    #SECTION 3: GENERATE INDEX LISTS AND SORT. DICTS ARE FOR REFERENCE
    #put the dict keys in lists and sort
    for key in index_dict_tr:
        index_list_tr.append(key)
    for key in index_dict_sr:
        index_list_sr.append(key)
    index_list_tr.sort()
    index_list_sr.sort()



    #SECTION 4: DIVIDE TRANSLATION UP INTO PARTIAL STRINGS AND REPLACE PARTS OF THE STRING ACCORDINGLY
    # mo1_count = 0
    # mo2_count = 0
    for i in range(0,len(index_list_tr)+1):

        if i == len(index_list_tr):
            temp_string = translated[index_tracker[i]::]
            if should_chop:
               if temp_string[0] == ' ':
                    temp_string = temp_string.replace(' ','',1)
            should_chop = False
            partial_strings.append(temp_string)
            break


        if index_dict_tr[index_list_tr[i]] == 'a':
            index_tracker.append(index_list_tr[i] + 5)
            temp_string = translated[index_tracker[i] : index_tracker[i+1]]
            #if the translation added a space before the target then chop it off
            if translated[index_list_tr[i]-1] == ' ' and source[index_list_sr[i]-1] != ' ':
                temp_string = temp_string.replace(' &amp;', '&amp;', 1)
            #should chop is method through which spaces are taken away from the end of previous targets
            if should_chop:
                if temp_string[0] == ' ':
                    temp_string = temp_string.replace(' ','',1)
                should_chop = False
            #if the translation added a space after the target then chop through bool(should_chop). Conversely if translation took away a space at the end then add it back.
            if len(translated)-1 >= index_list_tr[i]+5 and len(source)-1 >= index_list_sr[i]+5:
                if translated[index_list_tr[i]+5] == ' ' and source[index_list_sr[i]+5] != ' ':
                    should_chop = True
                if translated[index_list_tr[i]+5] != ' ' and source[index_list_sr[i]+5] == ' ':
                    temp_string += ' '
            #if the translation took away a space before the target then add it back
            if translated[index_list_tr[i]-1] != ' ' and source[index_list_sr[i]-1] == ' ':
                new_string = ''
                index = temp_string.index('&amp;')
                for z in range(0,len(temp_string)):
                    if z != index:
                        new_string += temp_string[z]
                    else:
                        new_string += ' '
                        new_string += temp_string[z]
                temp_string = new_string
            #add formatted string to partial strings list
            partial_strings.append(temp_string)



        if index_dict_tr[index_list_tr[i]] == 'ga':
            index_tracker.append(index_list_tr[i] + 4)
            temp_string = translated[index_tracker[i]:index_tracker[i+1]]
            #if the translation added a space before the target then chop it off
            if translated[index_list_tr[i]-1] == ' ' and source[index_list_sr[i]-1] != ' ':
                temp_string = temp_string.replace(' &gt;', '&gt.', 1)
            #should chop is method through which spaces are taken away from the end of previous targets
            if should_chop:
                if temp_string[0] == ' ':
                    temp_string = temp_string.replace(' ', '', 1)
                should_chop = False
            #if the translation added a space after the target then chop through bool(should_chop). Conversely if translation took away a space at the end then add it back.
            if len(translated)-1 >= index_list_tr[i]+4 and len(source)-1 >= index_list_sr[i]+4:
                if translated[index_list_tr[i]+4] == ' ' and source[index_list_sr[i]+4] != ' ':
                    should_chop = True
                if translated[index_list_tr[i]+4] != ' ' and source[index_list_sr[i]+4] == ' ':
                    temp_string += ' '
            #if the translation took away a space before the target then add it back
            if translated[index_list_tr[i]-1] != ' ' and source[index_list_sr[i]-1] == ' ':
                new_string = ''
                index = temp_string.index('&gt;')
                for z in range(0,len(temp_string)):
                    if z != index:
                        new_string += temp_string[z]
                    else:
                        new_string += ' '
                        new_string += temp_string[z]
                temp_string = new_string
            #add formatted string to partial strings list
            partial_strings.append(temp_string)




        if index_dict_tr[index_list_tr[i]] == 'gu':
            index_tracker.append(index_list_tr[i] + 6)
            temp_string = translated[index_tracker[i]:index_tracker[i+1]]
            temp_string = temp_string.replace('\\u003e', '&gt;')
            #if the translation added a space before the target then chop it off
            if translated[index_list_tr[i]-1] == ' ' and source[index_list_sr[i]-1] != ' ':
                temp_string = temp_string.replace(' &gt;', '&gt;', 1)
            #should chop is method through which spaces are taken away from the end of previous targets
            if should_chop:
                if temp_string[0] == ' ':
                    temp_string = temp_string.replace(' ','',1)
                should_chop = False
            #if the translation added a space after the target then chop through bool(should_chop). Conversely if translation took away a space at the end then add it back.
            if len(translated)-1 >= index_list_tr[i]+6 and len(source)-1 >= index_list_sr[i]+6:
                if translated[index_list_tr[i]+6] == ' ' and source[index_list_sr[i]+6] != ' ':
                    should_chop = True
                if translated[index_list_tr[i]+6] != ' ' and source[index_list_sr[i]+6] == ' ':
                    temp_string += ' '
            #if the translation took away a space before the target then add it back
            if translated[index_list_tr[i]-1] != ' ' and source[index_list_sr[i]-1] == ' ':
                new_string = ''
                index = temp_string.index('&gt;')
                for z in range(0,len(temp_string)):
                    if z != index:
                        new_string += temp_string[z]
                    else:
                        new_string += ' '
                        new_string += temp_string[z]
                temp_string = new_string
            #add formatted string to partial strings list
            partial_strings.append(temp_string)



        if index_dict_tr[index_list_tr[i]] == 'lu':
            index_tracker.append(index_list_tr[i] + 6)
            temp_string = translated[index_tracker[i]:index_tracker[i+1]]
            temp_string = temp_string.replace('\\u003c', '&lt;')
            #if the translation added a space before the target then chop it off
            if translated[index_list_tr[i]-1] == ' ' and source[index_list_sr[i]-1] != ' ':
                temp_string = temp_string.replace(' &lt;', '&lt;', 1)
            #should chop is method through which spaces are taken away from the end of previous targets
            if should_chop:
                if temp_string[0] == ' ':
                    temp_string = temp_string.replace(' ','',1)
                should_chop = False
            #if the translation added a space after the target then chop through bool(should_chop). Conversely if translation took away a space at the end then add it back.
            if len(translated)-1 >= index_list_tr[i]+6 and len(source)-1 >= index_list_sr[i]+6:
                if translated[index_list_tr[i]+6] == ' ' and source[index_list_sr[i]+6] != ' ':
                    should_chop = True
                if translated[index_list_tr[i]+6] != ' ' and source[index_list_sr[i]+6] == ' ':
                    temp_string += ' '
            #if the translation took away a space before the target then add it back
            if translated[index_list_tr[i]-1] != ' ' and source[index_list_sr[i]-1] == ' ':
                new_string = ''
                index = temp_string.index('&lt;')
                for z in range(0,len(temp_string)):
                    if z != index:
                        new_string += temp_string[z]
                    else:
                        new_string += ' '
                        new_string += temp_string[z]
                temp_string = new_string
            #add formatted string to partial strings list
            partial_strings.append(temp_string)



        if index_dict_tr[index_list_tr[i]] == 'la':
            index_tracker.append(index_list_tr[i] + 4)
            temp_string = translated[index_tracker[i]:index_tracker[i+1]]
            #if the translation added a space before the target then chop it off
            if translated[index_list_tr[i]-1] == ' ' and source[index_list_sr[i]-1] != ' ':
                temp_string = temp_string.replace(' &lt;', '&lt;', 1)
            #should chop is method through which spaces are taken away from the end of previous targets
            if should_chop:
                if temp_string[0] == ' ':
                    temp_string = temp_string.replace(' ','',1)
                should_chop = False
            #if the translation added a space after the target then chop through bool(should_chop). Conversely if translation took away a space at the end then add it back.
            if len(translated)-1 >= index_list_tr[i]+4 and len(source)-1 >= index_list_sr[i]+4:
                if translated[index_list_tr[i]+4] == ' ' and source[index_list_sr[i]+4] != ' ':
                    should_chop = True
                if translated[index_list_tr[i]+4] != ' ' and source[index_list_sr[i]+4] == ' ':
                    temp_string += ' '
            #if the translation took away a space before the target then add it back
            if translated[index_list_tr[i]-1] != ' ' and source[index_list_sr[i]-1] == ' ':
                new_string = ''
                index = temp_string.index('&lt;')
                for z in range(0,len(temp_string)):
                    if z != index:
                        new_string += temp_string[z]
                    else:
                        new_string += ' '
                        new_string += temp_string[z]
                temp_string = new_string
            #add formatted string to partial strings list
            partial_strings.append(temp_string)



        # if index_dict_tr[index_list_tr[i]] == 'n1':
        #     index_tracker.append(index_list_tr[i]+5)
        #     temp_string = translated[index_tracker[i]:index_tracker[i+1]]
        #     # #if the translation added a space before the target then chop it off
        #     # if translated[index_list_tr[i]-1] == ' ' and source[index_list_sr[i]-1] != ' ':
        #     #     temp_string = temp_string.replace(' ' + mo1[mo1_count] , mo1[mo1_count], 1)
        #     #should chop is method through which spaces are taken away from the end of previous targets
        #     if should_chop:
        #         if temp_string[0] == ' ':
        #             temp_string = temp_string.replace(' ','',1)
        #         should_chop = False
        #     # #if the translation added a space after the target then chop through bool(should_chop). Conversely if translation took away a space at the end then add it back.
        #     # if len(translated)-1 >= index_list_tr[i]+5 and len(source)-1 >= index_list_sr[i]+5:
        #     #     if translated[index_list_tr[i]+5] == ' ' and source[index_list_sr[i]+5] != ' ':
        #     #         should_chop = True
        #     #     if translated[index_list_tr[i]+5] != ' ' and source[index_list_sr[i]+5] == ' ':
        #     #         temp_string += ' '
        #     # #if the translation took away a space before the target then add it back
        #     # if translated[index_list_tr[i]-1] != ' ' and source[index_list_sr[i]-1] == ' ':
        #     #     new_string = ''
        #     #     index = temp_string.index(mo1[mo1_count])
        #     #     for z in range(0,len(temp_string)):
        #     #         if z != index:
        #     #             new_string += temp_string[z]
        #     #         else:
        #     #             new_string += ' '
        #     #             new_string += temp_string[z]
        #     #     temp_string = new_string
        #     # #add formatted string to partial strings list
        #     partial_strings.append(temp_string)
        #     # mo1_count += 1



        # if index_dict_tr[index_list_tr[i]] == 'n2':
        #     index_tracker.append(index_list_tr[i]+6)
        #     temp_string = translated[index_tracker[i]:index_tracker[i+1]]
        #     # #if the translation added a space before the target then chop it off
        #     # if translated[index_list_tr[i]-1] == ' ' and source[index_list_sr[i]-1] != ' ':
        #     #     temp_string = temp_string.replace(' ' + mo2[mo2_count] , mo2[mo2_count], 1)
        #     #should chop is method through which spaces are taken away from the end of previous targets
        #     if should_chop:
        #         if temp_string[0] == ' ':
        #             temp_string = temp_string.replace(' ','',1)
        #         should_chop = False
        #     # #if the translation added a space after the target then chop through bool(should_chop). Conversely if translation took away a space at the end then add it back.
        #     # if len(translated)-1 >= index_list_tr[i]+6 and len(source)-1 >= index_list_sr[i]+6:
        #     #     if translated[index_list_tr[i]+6] == ' ' and source[index_list_sr[i]+6] != ' ':
        #     #         should_chop = True
        #     #     if translated[index_list_tr[i]+6] != ' ' and source[index_list_sr[i]+6] == ' ':
        #     #         temp_string += ' '
        #     # #if the translation took away a space before the target then add it back
        #     # if translated[index_list_tr[i]-1] != ' ' and source[index_list_sr[i]-1] == ' ':
        #     #     new_string = ''
        #     #     index = temp_string.index(mo2[mo2_count])
        #     #     for z in range(0,len(temp_string)):
        #     #         if z != index:
        #     #             new_string += temp_string[z]
        #     #         else:
        #     #             new_string += ' '
        #     #             new_string += temp_string[z]
        #     #     temp_string = new_string
        #     # #add formatted string to partial strings list
        #     partial_strings.append(temp_string)
        #     # mo2_count += 1


    #SECTION 5: GENERATE OUTPUT
    for string in partial_strings:
        output_string += string


    return output_string






#This function is the one used in the translate.xslt file
def translate(context, phrase, lang = 'fr' ): # will have to get information from locale

    # unreserved_characters = ['a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'
    #                          'A','B','C','D','E','F','G','H','I','J','K','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    #                          '0','1','2','3','4','5','6','7','8','9','-','_','.','~']

    reserved_characters = [' ','!','*','\'','(',')',';',':','@','&','=','+','$',',','/','?','%','#','[',']']

    base_url = 'https://www.googleapis.com/language/translate/v2?key='
    API_key = '' #manually insert API key
    source_language = 'source=en'
    translation_language = 'target=' + lang
    formatted_phrase = ''

    for letter in phrase:
        if letter in reserved_characters:
            formatted_phrase += quote(letter,safe="")
        else:
            formatted_phrase += letter

    url = base_url + API_key + '&' + source_language + '&' + translation_language + '&q=' + formatted_phrase

    try:
        stream = urllib.request.urlopen(url)
    except:
        return ""

    #The json output of the request to google may contain more than one string that matches the regex. The correct one to use is always first. Other matches give irrelevant information.
    translation_regex = re.compile(r'"translatedText":\s"(.*)"', re.IGNORECASE)
    translated_text = []
    mo = ''
    for line in stream:
        line = line.decode('UTF-8')
        mo = translation_regex.findall(line)
        if len(mo) != 0:
            translated_text.append(mo[0])
    print(phrase)

    translated_string = translated_text[0]
    print(translated_string)

    #Since values get unencoded when reading the source text in xml files, reencode the ones that are used in the space_handler() function
    source_text = phrase
    source_text = source_text.replace('&', '&amp;')
    source_text = source_text.replace('<', '&lt;')
    source_text = source_text.replace('>', '&gt;')
    print(source_text)

    #reformat spaces in translated string to match those in the source text
    translated_string = space_handler(source_text, translated_string)
    print(translated_string)
    print()


    return translated_string


ns = etree.FunctionNamespace("mynamespace")
ns['translate'] = translate

dom = etree.parse(r'C:\Users\Andrew\PycharmProjects\xTuple\fr.ts.xml')

xslt = etree.parse("translate.xslt")
transform = etree.XSLT(xslt)

newdom = transform(dom)

filename = 'output.xml'
output_xml = open(filename,'w')
xml_declaration = '<?xml version="1.0" encoding="utf-8"?>\n'
output_xml.write(xml_declaration)
output_xml.write(str(newdom))
output_xml.close()



